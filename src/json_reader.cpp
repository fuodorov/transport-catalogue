#include "json_reader.h"

namespace transport_catalogue {
namespace detail {
namespace json {
    
JSONReader::JSONReader(Document doc) : document_(std::move(doc)) {}
JSONReader::JSONReader(std::istream& input) : document_(json::load(input)) {}    
    
Stop JSONReader::parse_node_stop(Node& node) {
    Stop stop;
    Dict stop_node;
    
    if (node.is_dict()) {
        stop_node = node.as_dict();
        stop.name = stop_node.at("name").as_string();
        stop.latitude = stop_node.at("latitude").as_double();
        stop.longitude = stop_node.at("longitude").as_double();
    }
    
    return stop;
}
    
std::vector<Distance> JSONReader::parse_node_distances(Node& node, TransportCatalogue& catalogue) {
    std::vector<Distance> distances;
    Dict stop_node;
    Dict stop_road_map;
    std::string begin_name;
    std::string last_name;
    int distance;
    
    if (node.is_dict()) {
        stop_node = node.as_dict();
        begin_name = stop_node.at("name").as_string();
        
        try {
            stop_road_map = stop_node.at("road_distances").as_dict();
            
            for (auto [key, value] : stop_road_map) {
                last_name = key;
                distance = value.as_int();
                distances.push_back({catalogue.get_stop(begin_name), 
                                     catalogue.get_stop(last_name), distance});
            } 
            
        } catch(...) {
            std::cout << "invalide road" << std::endl;
        }        
    }
    
    return distances;
}

Bus JSONReader::parse_node_bus(Node& node, TransportCatalogue& catalogue) {
    Bus bus;
    Dict bus_node;
    Array bus_stops;
    
    if (node.is_dict()) {
        bus_node = node.as_dict();
        bus.name = bus_node.at("name").as_string();
        bus.is_roundtrip = bus_node.at("is_roundtrip").as_bool();

        try {
            bus_stops = bus_node.at("stops").as_array();
            
            for (Node stop : bus_stops) {
                bus.stops.push_back(catalogue.get_stop(stop.as_string()));
            }

            if (!bus.is_roundtrip) {
                size_t size = bus.stops.size() - 1;
                
                for (size_t i = size; i > 0; i--) {
                    bus.stops.push_back(bus.stops[i-1]);
                }
                
            } 
            
        } catch(...) {
            std::cout << "base_requests: bus: stops is empty" << std::endl;
        }        
    }
    
    return bus;
}
    
void JSONReader::parse_node_base(const Node& root, TransportCatalogue& catalogue){
    Array base_requests;
    Dict req_map;
    Node req_node;
    
    std::vector<Node> buses;
    std::vector<Node> stops;
    
    if (root.is_array()) {
        base_requests = root.as_array();
        
        for (Node& node : base_requests) {            
            if (node.is_dict()) {
                req_map = node.as_dict();
                
                try {
                    req_node = req_map.at("type");
                    
                    if (req_node.is_string()) {
                        
                        if (req_node.as_string() == "Bus") {
                            buses.push_back(req_map);
                        } else if (req_node.as_string() == "Stop") {
                            stops.push_back(req_map);
                        } else {
                            std::cout << "base_requests are invalid";
                        }
                    }
                    
                } catch(...) {
                    std::cout << "base_requests does not have type value";
                }                
            }
        }   
        
        for (auto stop : stops) {
            catalogue.add_stop(parse_node_stop(stop));
        }
        
        for (auto stop : stops) {
            catalogue.add_distance(parse_node_distances(stop, catalogue));
        }
        
        for (auto bus : buses) {
            catalogue.add_bus(parse_node_bus(bus, catalogue));
        }
        
    } else {
        std::cout << "base_requests is not an array";
    }  
}

void JSONReader::parse_node_stat(const Node& node, std::vector<StatRequest>& stat_request){
    Array stat_requests;
    Dict req_map;
    StatRequest req;
    
    if (node.is_array()) {
        stat_requests = node.as_array();
        
        for (Node req_node : stat_requests) {
            
            if (req_node.is_dict()) {
                req_map = req_node.as_dict();
                req.id = req_map.at("id").as_int();
                req.type = req_map.at("type").as_string();               

                if ((req.type == "Bus") || (req.type == "Stop")) {
                    req.name = req_map.at("name").as_string();
                    req.from ="";
                    req.to = "";
                    
                } else {
                    req.name = "";
                    if (req.type == "Route") {
                        req.from = req_map.at("from").as_string();
                        req.to = req_map.at("to").as_string();
                        
                    } else {
                        req.from ="";
                        req.to = "";
                    }
                }
                
                stat_request.push_back(req);                
            }   
        } 
        
    } else {
        std::cout << "stat_requests is not array";
    }
}
  
void JSONReader::parse_node_render(const Node& node, map_renderer::RenderSettings& rend_set){
    Dict rend_map;
    Array bus_lab_offset;
    Array stop_lab_offset;
    Array arr_color;
    Array arr_palette;
    uint8_t red_;
    uint8_t green_;
    uint8_t blue_;
    double opacity_;

    if (node.is_dict()) {
        rend_map = node.as_dict();
        
        try {
            rend_set.width_ = rend_map.at("width").as_double();
            rend_set.height_ = rend_map.at("height").as_double();
            rend_set.padding_ = rend_map.at("padding").as_double();
            rend_set.line_width_ = rend_map.at("line_width").as_double();
            rend_set.stop_radius_ = rend_map.at("stop_radius").as_double();
            
            rend_set.bus_label_font_size_ = rend_map.at("bus_label_font_size").as_int();
            
            if (rend_map.at("bus_label_offset").is_array()) {
                bus_lab_offset = rend_map.at("bus_label_offset").as_array();
                rend_set.bus_label_offset_ = std::make_pair(bus_lab_offset[0].as_double(),
                                                            bus_lab_offset[1].as_double());
            }
            
            rend_set.stop_label_font_size_ = rend_map.at("stop_label_font_size").as_int();

            if (rend_map.at("stop_label_offset").is_array()) {
                stop_lab_offset = rend_map.at("stop_label_offset").as_array();
                rend_set.stop_label_offset_ = std::make_pair(stop_lab_offset[0].as_double(),
                                                             stop_lab_offset[1].as_double());
            }
            
            if (rend_map.at("underlayer_color").is_string()) {
                rend_set.underlayer_color_ = svg::Color(rend_map.at("underlayer_color").as_string());
            } else if (rend_map.at("underlayer_color").is_array()) {
                arr_color = rend_map.at("underlayer_color").as_array();
                red_ = arr_color[0].as_int();
                green_ = arr_color[1].as_int();
                blue_ = arr_color[2].as_int();

                if(arr_color.size() == 4){
                    opacity_ = arr_color[3].as_double();
                    rend_set.underlayer_color_ = svg::Color(svg::Rgba(red_, 
                                                                      green_, 
                                                                      blue_, 
                                                                      opacity_));
                } else if (arr_color.size() == 3) {
                    rend_set.underlayer_color_ = svg::Color(svg::Rgb(red_, 
                                                                     green_, 
                                                                     blue_));
                }
                
            }

            rend_set.underlayer_width_ = rend_map.at("underlayer_width").as_double();

            if (rend_map.at("color_palette").is_array()) {
                arr_palette = rend_map.at("color_palette").as_array();
                
                for (Node color_palette : arr_palette) {
                    
                    if (color_palette.is_string()) {
                        rend_set.color_palette_.push_back(svg::Color(color_palette.as_string()));
                    } else if (color_palette.is_array()) {
                        arr_color = color_palette.as_array();
                        red_ = arr_color[0].as_int();
                        green_ = arr_color[1].as_int();
                        blue_ = arr_color[2].as_int();

                        if (arr_color.size() == 4) {
                            opacity_ = arr_color[3].as_double();
                            rend_set.color_palette_.push_back(svg::Color(svg::Rgba(red_, 
                                                                                   green_, 
                                                                                   blue_, 
                                                                                   opacity_)));
                        } else if (arr_color.size() == 3) {
                            rend_set.color_palette_.push_back(svg::Color(svg::Rgb(red_, 
                                                                                  green_, 
                                                                                  blue_)));
                        }
                    }
                }
            }            
        } catch(...) {
            std::cout << "unable to parsse init settings";
        }
        
    } else {
        std::cout << "render_settings is not map";
    }
}
    
void JSONReader::parse_node_routing(const Node& node, router::RoutingSettings& route_set) {
    Dict route;
    
    if (node.is_dict()) {
        route = node.as_dict();

        try {

            route_set.bus_wait_time = route.at("bus_wait_time").as_double();
            route_set.bus_velocity = route.at("bus_velocity").as_double();
            
        } catch(...) {
            std::cout << "unable to parse routing settings";
        }
        
    } else {
        std::cout << "routing settings is not map";
    }

}
    
void JSONReader::parse_node_serialization(const Node& node, serialization::SerializationSettings& serialization_set) {
    
    Dict serialization;

    if (node.is_dict()) {
        serialization = node.as_dict();

        try {
            serialization_set.file_name = serialization.at("file").as_string();

        } catch(...) {
            std::cout << "unable to parse serialization settings";
        }

    } else {
        std::cout << "serialization settings is not map";
    }
}
    
void JSONReader::parse_node_make_base(TransportCatalogue& catalogue, 
                                      map_renderer::RenderSettings& render_settings, 
                                      router::RoutingSettings& routing_settings,
                                      serialization::SerializationSettings& serialization_settings) { 
    Dict root_dictionary;
    
    if (document_.get_root().is_dict()) {
        root_dictionary = document_.get_root().as_dict();
        
        try {          
            parse_node_base(root_dictionary.at("base_requests"), catalogue);
            
        } catch(...) {}   
        
        try {
            parse_node_render(root_dictionary.at("render_settings"), render_settings);
            
        } catch(...) {}
        
        try {
            parse_node_routing(root_dictionary.at("routing_settings"), routing_settings);
            
        } catch(...) {}
        
        try {
            parse_node_serialization(root_dictionary.at("serialization_settings"), serialization_settings);
            
        } catch(...) {}
        
    } else {
        std::cout << "root is not map";
    }
}
    
void JSONReader::parse_node_process_requests(std::vector<StatRequest>& stat_request,
                                             serialization::SerializationSettings& serialization_settings) { 
    Dict root_dictionary;

    if (document_.get_root().is_dict()) {
        root_dictionary = document_.get_root().as_dict();

        try {
            parse_node_stat(root_dictionary.at("stat_requests"), stat_request);
            
        } catch(...) {}

        try {
            parse_node_serialization(root_dictionary.at("serialization_settings"), serialization_settings);
            
        } catch(...) {}

    } else {
        std::cout << "root is not map";
    }
}
    
}//end namespace json
}//end namespace detail
}//end namespace transport_catalogue
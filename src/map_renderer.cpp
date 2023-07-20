#include "map_renderer.h"

namespace map_renderer {

bool SphereProjector::is_zero(double value) {
    return std::abs(value) < EPSILON;
}
    
MapRenderer::MapRenderer(RenderSettings& render_settings) : render_settings_(render_settings) {}

svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
    return {(coords.longitude - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.latitude) * zoom_coeff_ + padding_};
}
    
SphereProjector MapRenderer::get_sphere_projector(const std::vector<geo::Coordinates>& points) const {
        return SphereProjector(points.begin(),
                               points.end(),
                               render_settings_.width_,
                               render_settings_.height_,
                               render_settings_.padding_);
}
  
void MapRenderer::init_sphere_projector(std::vector<geo::Coordinates> points){
    sphere_projector = SphereProjector(points.begin(),
                               points.end(),
                               render_settings_.width_,
                               render_settings_.height_,
                               render_settings_.padding_);
}
    
RenderSettings MapRenderer::get_render_settings() const{
    return render_settings_;
}
  
int MapRenderer::get_palette_size() const{
    return render_settings_.color_palette_.size();
}
    
svg::Color MapRenderer::get_color(int line_number) const {
    return render_settings_.color_palette_[line_number];
}

void MapRenderer::set_line_properties(svg::Polyline& polyline, 
                                      [[maybe_unused]] int line_number) const {
    using namespace std::literals;
    
    polyline.set_stroke_color(get_color(line_number));   
    polyline.set_fill_color("none"s);
    polyline.set_stroke_width(render_settings_.line_width_);
    polyline.set_stroke_linecap(svg::StrokeLineCap::ROUND);
    polyline.set_stroke_linejoin(svg::StrokeLineJoin::ROUND);   
} 
    
void MapRenderer::set_route_text_common_properties(svg::Text& text, 
                                                   const std::string& name, 
                                                   svg::Point position) const {
    using namespace std::literals;

    text.set_position(position);
    text.set_offset({render_settings_.bus_label_offset_.first,
                     render_settings_.bus_label_offset_.second});
    text.set_font_size(render_settings_.bus_label_font_size_);
    text.set_font_family("Verdana");
    text.set_font_weight("bold");
    text.set_data(name);   
} 
    
void MapRenderer::set_route_text_additional_properties(svg::Text& text, 
                                                       const std::string& name, 
                                                       svg::Point position) const {
    set_route_text_common_properties(text, 
                                     name, 
                                     position);
    
    text.set_fill_color(render_settings_.underlayer_color_);
    text.set_stroke_color(render_settings_.underlayer_color_);
    text.set_stroke_width(render_settings_.underlayer_width_);
    text.set_stroke_linejoin(svg::StrokeLineJoin::ROUND);
    text.set_stroke_linecap(svg::StrokeLineCap::ROUND);
}

void MapRenderer::set_route_text_color_properties(svg::Text& text, 
                                                  const std::string& name, 
                                                  int palette, 
                                                  svg::Point position) const {
    set_route_text_common_properties(text, 
                                     name, 
                                     position);
    
    text.set_fill_color(get_color(palette));
}
    
void MapRenderer::set_stops_circles_properties(svg::Circle& circle, 
                                               svg::Point position) const {
    using namespace std::literals;
    
    circle.set_center(position);
    circle.set_radius(render_settings_.stop_radius_);
    circle.set_fill_color("white");
} 
    
void MapRenderer::set_stops_text_common_properties(svg::Text& text, 
                                                   const std::string& name, 
                                                   svg::Point position) const {
    using namespace std::literals;

    text.set_position(position);
    text.set_offset({render_settings_.stop_label_offset_.first,
                     render_settings_.stop_label_offset_.second});
    text.set_font_size(render_settings_.stop_label_font_size_);
    text.set_font_family("Verdana");
    text.set_data(name);
}
 
void MapRenderer::set_stops_text_additional_properties(svg::Text& text, 
                                                       const std::string& name, 
                                                       svg::Point position) const {
    using namespace std::literals;
    set_stops_text_common_properties(text, 
                                     name, 
                                     position);

    text.set_fill_color(render_settings_.underlayer_color_);
    text.set_stroke_color(render_settings_.underlayer_color_);
    text.set_stroke_width(render_settings_.underlayer_width_);
    text.set_stroke_linejoin(svg::StrokeLineJoin::ROUND);
    text.set_stroke_linecap(svg::StrokeLineCap::ROUND);
}
 
void MapRenderer::set_stops_text_color_properties(svg::Text& text, 
                                                  const std::string& name, 
                                                  svg::Point position) const {
    using namespace std::literals;

    set_stops_text_common_properties(text, 
                                     name, 
                                     position);
    text.set_fill_color("black");
}
    
void MapRenderer::add_line(std::vector<std::pair<Bus*, int>>& buses_palette) {    
    std::vector<geo::Coordinates> stops_geo_coords;
    
    for (auto [bus, palette] : buses_palette) { 
        
        for (Stop* stop : bus->stops) {
            geo::Coordinates coordinates;
            coordinates.latitude = stop->latitude;
            coordinates.longitude = stop->longitude;
            
            stops_geo_coords.push_back(coordinates); 
        }
        
        svg::Polyline bus_line;
        bool bus_empty = true;   
        
        for (auto& coord : stops_geo_coords) {
            bus_empty = false;
            bus_line.add_point(sphere_projector(coord));
        } 
        
        if (!bus_empty) {
            set_line_properties(bus_line, 
                                palette);
            map_svg.add(bus_line);   
        }
        
        stops_geo_coords.clear();
    }
}
    
void MapRenderer::add_buses_name(std::vector<std::pair<Bus*, int>>& buses_palette){    
    std::vector<geo::Coordinates> stops_geo_coords;
    bool bus_empty = true; 
    
    for (auto [bus, palette] : buses_palette) {  
        
        for (Stop* stop : bus->stops) {
            geo::Coordinates coordinates;
            coordinates.latitude = stop->latitude;
            coordinates.longitude = stop->longitude;
            
            stops_geo_coords.push_back(coordinates); 
            
            if(bus_empty) bus_empty = false;
        }
        
        svg::Text route_name_roundtrip;
        svg::Text route_title_roundtrip;
        svg::Text route_name_notroundtrip;
        svg::Text route_title_notroundtrip; 
        
        if (!bus_empty) {
            
            if (bus->is_roundtrip) {
                set_route_text_additional_properties(route_name_roundtrip,
                                                     std::string(bus->name),
                                                     sphere_projector(stops_geo_coords[0]));
                map_svg.add(route_name_roundtrip);
                
                set_route_text_color_properties(route_title_roundtrip,
                                                std::string(bus->name),
                                                palette,
                                                sphere_projector(stops_geo_coords[0]));
                map_svg.add(route_title_roundtrip);
                
            } else {
                set_route_text_additional_properties(route_name_roundtrip,
                                                     std::string(bus->name),
                                                     sphere_projector(stops_geo_coords[0]));
                map_svg.add(route_name_roundtrip);
                
                set_route_text_color_properties(route_title_roundtrip,
                                                std::string(bus->name),
                                                palette,
                                                sphere_projector(stops_geo_coords[0]));
                map_svg.add(route_title_roundtrip);
                
                if (stops_geo_coords[0] != stops_geo_coords[stops_geo_coords.size()/2]) {
                    set_route_text_additional_properties(route_name_notroundtrip,
                                                         std::string(bus->name),
                                                         sphere_projector(stops_geo_coords[stops_geo_coords.size()/2]));
                    map_svg.add(route_name_notroundtrip);
                    
                    set_route_text_color_properties(route_title_notroundtrip,
                                                    std::string(bus->name),
                                                    palette,
                                                    sphere_projector(stops_geo_coords[stops_geo_coords.size()/2]));
                    map_svg.add(route_title_notroundtrip);
                }
            } 
        }
        
        bus_empty = false;
        stops_geo_coords.clear();
    }
}
    
void MapRenderer::add_stops_circle(std::vector<Stop*>& stops){
    std::vector<geo::Coordinates> stops_geo_coords;      
    svg::Circle icon;
    
    for (Stop* stop_info : stops) { 
        
        if (stop_info) {
            geo::Coordinates coordinates;
            coordinates.latitude = stop_info->latitude;
            coordinates.longitude = stop_info->longitude; 

            set_stops_circles_properties(icon, 
                                         sphere_projector(coordinates));
            map_svg.add(icon);  
        }
    }
}
  
void MapRenderer::add_stops_name(std::vector<Stop*>& stops){    
    std::vector<geo::Coordinates> stops_geo_coords;    
    
    svg::Text svg_stop_name;
    svg::Text svg_stop_name_title;
    
    for (Stop* stop_info : stops) {
        
        if (stop_info) {
            geo::Coordinates coordinates;
            coordinates.latitude = stop_info->latitude;
            coordinates.longitude = stop_info->longitude;            
            
            set_stops_text_additional_properties(svg_stop_name, 
                                                 stop_info->name, 
                                                 sphere_projector(coordinates));
            map_svg.add(svg_stop_name);
            
            set_stops_text_color_properties(svg_stop_name_title, 
                                            stop_info->name, 
                                            sphere_projector(coordinates));
            map_svg.add(svg_stop_name_title); 
        }
    }
}
  
void MapRenderer::get_stream_map(std::ostream& stream_) { 
    map_svg.render(stream_);
}
    
}//end namespace map_renderer
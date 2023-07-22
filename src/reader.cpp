#include "reader.h"

namespace transport_catalogue {
namespace json {

Parser::Parser(Document doc) : document(std::move(doc)) {}
Parser::Parser(std::istream &input) : document(json::Load(input)) {}

Stop Parser::ProcessNodeStop(Node &node) {
  return node.IsDict() ? Stop{node.AsDict().at("name").AsString(),
                              node.AsDict().at("latitude").AsDouble(),
                              node.AsDict().at("longitude").AsDouble()}
                       : Stop();
}

std::vector<Distance> Parser::ProcessNodeDistances(
    Node &node, TransportCatalogue &catalogue) {
  std::vector<Distance> distances;

  if (node.IsDict()) {
    try {
      for (auto [key, value] : node.AsDict().at("road_distances").AsDict()) {
        distances.push_back(
            {catalogue.GetStop(node.AsDict().at("name").AsString()),
             catalogue.GetStop(key), value.AsInt()});
      }

    } catch (...) {
      std::cout << "invalide road" << std::endl;
    }
  }

  return distances;
}

Bus Parser::ProcessNodeBus(Node &node, TransportCatalogue &catalogue) {
  Bus bus;

  if (node.IsDict()) {
    bus.name = node.AsDict().at("name").AsString();
    bus.is_round_trip = node.AsDict().at("is_round_trip").AsBool();

    try {
      for (Node stop : node.AsDict().at("stops").AsArray()) {
        bus.stops.push_back(catalogue.GetStop(stop.AsString()));
      }

      if (!bus.is_round_trip) {
        size_t size = bus.stops.size() - 1;

        for (size_t i = size; i > 0; i--) {
          bus.stops.push_back(bus.stops[i - 1]);
        }
      }

    } catch (...) {
      std::cout << "invalide bus" << std::endl;
    }
  }

  return bus;
}

void Parser::ProcessNodeTransportCatalogue(const Node &root,
                                           TransportCatalogue &catalogue) {
  std::vector<Node> buses, stops;

  if (root.IsArray()) {
    for (Node node : root.AsArray()) {
      if (node.IsDict()) {
        try {
          if (node.AsDict().at("type").IsString()) {
            if (node.AsDict().at("type").AsString() == "Bus") {
              buses.push_back(node.AsDict());
            } else if (node.AsDict().at("type").AsString() == "Stop") {
              stops.push_back(node.AsDict());
            } else {
              std::cout << "base_requests are invalid" << std::endl;
            }
          }
        } catch (...) {
          std::cout << "base_requests does not have type value" << std::endl;
        }
      }
    }

    for (auto stop : stops) {
      catalogue.AddStop(ProcessNodeStop(stop));
    }

    for (auto stop : stops) {
      catalogue.AddDistance(ProcessNodeDistances(stop, catalogue));
    }

    for (auto bus : buses) {
      catalogue.AddBus(ProcessNodeBus(bus, catalogue));
    }

  } else {
    std::cout << "base_requests is not an array" << std::endl;
  }
}

void Parser::ProcessNodeStatisticRequest(
    const Node &node, std::vector<StatisticRequest> &stat_request) {
  StatisticRequest tmp_stat_request;

  if (node.IsArray()) {
    for (Node tmp_node : node.AsArray()) {
      if (tmp_node.IsDict()) {
        tmp_stat_request.id = tmp_node.AsDict().at("id").AsInt();
        tmp_stat_request.type = tmp_node.AsDict().at("type").AsString();

        if ((tmp_stat_request.type == "Bus") ||
            (tmp_stat_request.type == "Stop")) {
          tmp_stat_request.name = tmp_node.AsDict().at("name").AsString();
          tmp_stat_request.from = "";
          tmp_stat_request.to = "";

        } else {
          tmp_stat_request.name = "";
          if (tmp_stat_request.type == "Route") {
            tmp_stat_request.from = tmp_node.AsDict().at("from").AsString();
            tmp_stat_request.to = tmp_node.AsDict().at("to").AsString();

          } else {
            tmp_stat_request.from = "";
            tmp_stat_request.to = "";
          }
        }

        stat_request.push_back(tmp_stat_request);
      }
    }

  } else {
    std::cout << "stat_requests is not array" << std::endl;
  }
}

void Parser::ProcessNodeRenderSettings(
    const Node &node, renderer::RenderSettings &render_settings) {
  if (node.IsDict()) {
    try {
      render_settings.width_ = node.AsDict().at("width").AsDouble();
      render_settings.height_ = node.AsDict().at("height").AsDouble();
      render_settings.padding_ = node.AsDict().at("padding").AsDouble();
      render_settings.line_width_ = node.AsDict().at("line_width").AsDouble();
      render_settings.stop_radius_ = node.AsDict().at("stop_radius").AsDouble();
      render_settings.bus_label_font_size_ =
          node.AsDict().at("bus_label_font_size").AsInt();

      if (node.AsDict().at("bus_label_offset").IsArray()) {
        render_settings.bus_label_offset_ = std::make_pair(
            node.AsDict().at("bus_label_offset").AsArray()[0].AsDouble(),
            node.AsDict().at("bus_label_offset").AsArray()[1].AsDouble());
      }

      render_settings.stop_label_font_size_ =
          node.AsDict().at("stop_label_font_size").AsInt();

      if (node.AsDict().at("stop_label_offset").IsArray()) {
        render_settings.stop_label_offset_ = std::make_pair(
            node.AsDict().at("stop_label_offset").AsArray()[0].AsDouble(),
            node.AsDict().at("stop_label_offset").AsArray()[1].AsDouble());
      }

      if (node.AsDict().at("underlayer_color").IsString()) {
        render_settings.underlayer_color_ =
            svg::Color(node.AsDict().at("underlayer_color").AsString());
      } else if (node.AsDict().at("underlayer_color").IsArray()) {
        if (node.AsDict().at("underlayer_color").AsArray().size() == 4) {
          render_settings.underlayer_color_ = svg::Color(svg::Rgba(
              node.AsDict().at("underlayer_color").AsArray()[0].AsInt(),
              node.AsDict().at("underlayer_color").AsArray()[1].AsInt(),
              node.AsDict().at("underlayer_color").AsArray()[2].AsInt(),
              node.AsDict().at("underlayer_color").AsArray()[3].AsDouble()));
        } else if (node.AsDict().at("underlayer_color").AsArray().size() == 3) {
          render_settings.underlayer_color_ = svg::Color(svg::Rgb(
              node.AsDict().at("underlayer_color").AsArray()[0].AsInt(),
              node.AsDict().at("underlayer_color").AsArray()[1].AsInt(),
              node.AsDict().at("underlayer_color").AsArray()[2].AsInt()));
        }
      }

      render_settings.underlayer_width_ =
          node.AsDict().at("underlayer_width").AsDouble();

      if (node.AsDict().at("color_palette").IsArray()) {
        for (Node color_palette : node.AsDict().at("color_palette").AsArray()) {
          if (color_palette.IsString()) {
            render_settings.color_palette_.push_back(
                svg::Color(color_palette.AsString()));
          } else if (color_palette.IsArray()) {
            if (node.AsDict().at("color_palette").AsArray().size() == 4) {
              render_settings.color_palette_.push_back(svg::Color(svg::Rgba(
                  color_palette.AsArray()[0].AsInt(),
                  color_palette.AsArray()[1].AsInt(),
                  color_palette.AsArray()[2].AsInt(),
                  node.AsDict().at("color_palette").AsArray()[3].AsDouble())));
            } else if (node.AsDict().at("color_palette").AsArray().size() ==
                       3) {
              render_settings.color_palette_.push_back(
                  svg::Color(svg::Rgb(color_palette.AsArray()[0].AsInt(),
                                      color_palette.AsArray()[1].AsInt(),
                                      color_palette.AsArray()[2].AsInt())));
            }
          }
        }
      }
    } catch (...) {
      std::cout << "unable to parsse init settings" << std::endl;
    }

  } else {
    std::cout << "render_settings is not map" << std::endl;
  }
}

void Parser::ProcessNodeRoutingSettings(
    const Node &node, router::RoutingSettings &route_settings) {
  if (node.IsDict()) {
    try {
      route_settings.bus_wait_time =
          node.AsDict().at("bus_wait_time").AsDouble();
      route_settings.bus_velocity = node.AsDict().at("bus_velocity").AsDouble();

    } catch (...) {
      std::cout << "unable to parse routing settings" << std::endl;
    }

  } else {
    std::cout << "routing settings is not map" << std::endl;
  }
}

void Parser::ProcessNodeSerializationSettings(
    const Node &node,
    serialization::SerializationSettings &serialization_settings) {
  if (node.IsDict()) {
    try {
      serialization_settings.file_name = node.AsDict().at("file").AsString();

    } catch (...) {
      std::cout << "unable to parse serialization settings" << std::endl;
    }

  } else {
    std::cout << "serialization settings is not map" << std::endl;
  }
}

void Parser::ProcessTransportCatalogue(
    TransportCatalogue &catalogue, renderer::RenderSettings &render_settings,
    router::RoutingSettings &routing_settings,
    serialization::SerializationSettings &serialization_settings) {
  if (document.GetRoot().IsDict()) {
    try {
      ProcessNodeTransportCatalogue(
          document.GetRoot().AsDict().at("base_requests"), catalogue);
      ProcessNodeRenderSettings(
          document.GetRoot().AsDict().at("render_settings"), render_settings);
      ProcessNodeRoutingSettings(
          document.GetRoot().AsDict().at("routing_settings"), routing_settings);
      ProcessNodeSerializationSettings(
          document.GetRoot().AsDict().at("serialization_settings"),
          serialization_settings);
    } catch (...) {
      std::cout << "unable to parse root" << std::endl;
    }

  } else {
    std::cout << "root is not map" << std::endl;
  }
}

void Parser::ProcessRequests(
    std::vector<StatisticRequest> &stat_request,
    serialization::SerializationSettings &serialization_settings) {
  if (document.GetRoot().IsDict()) {
    try {
      ProcessNodeStatisticRequest(
          document.GetRoot().AsDict().at("stat_requests"), stat_request);
      ProcessNodeSerializationSettings(
          document.GetRoot().AsDict().at("serialization_settings"),
          serialization_settings);

    } catch (...) {
      std::cout << "unable to parse root" << std::endl;
    }

  } else {
    std::cout << "root is not map" << std::endl;
  }
}

}  // end namespace json
}  // end namespace transport_catalogue
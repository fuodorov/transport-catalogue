#include "serializer.h"

namespace serialization {

template <typename It>
uint32_t calculate_id(It start, It end, std::string_view name) {
  auto stop_it = std::find_if(start, end, [&name](const domain::Stop stop) {
    return stop.name == name;
  });
  return std::distance(start, stop_it);
}

transport_catalogue_protobuf::TransportCatalogue
transport_catalogue_serialization(
    const transport_catalogue::TransportCatalogue &transport_catalogue) {
  transport_catalogue_protobuf::TransportCatalogue transport_catalogue_proto;

  const auto &stops = transport_catalogue.GetStops();
  const auto &buses = transport_catalogue.GetBuses();
  const auto &distances = transport_catalogue.GetDistance();

  int id = 0;
  for (const auto &stop : stops) {
    transport_catalogue_protobuf::Stop stop_proto;

    stop_proto.set_id(id);
    stop_proto.set_name(stop.name);
    stop_proto.set_latitude(stop.latitude);
    stop_proto.set_longitude(stop.longitude);

    *transport_catalogue_proto.add_stops() = std::move(stop_proto);

    ++id;
  }

  for (const auto &bus : buses) {
    transport_catalogue_protobuf::Bus bus_proto;

    bus_proto.set_name(bus.name);

    for (auto stop : bus.stops) {
      uint32_t stop_id = calculate_id(stops.cbegin(), stops.cend(), stop->name);
      bus_proto.add_stops(stop_id);
    }

    bus_proto.set_is_round_trip(bus.is_round_trip);
    bus_proto.set_route_length(bus.route_length);

    *transport_catalogue_proto.add_buses() = std::move(bus_proto);
  }

  for (const auto &[stops_pair, pair_distance] : distances) {
    transport_catalogue_protobuf::Distance distance_proto;

    distance_proto.set_start(
        calculate_id(stops.cbegin(), stops.cend(), stops_pair.first->name));

    distance_proto.set_end(
        calculate_id(stops.cbegin(), stops.cend(), stops_pair.second->name));

    distance_proto.set_distance(pair_distance);

    *transport_catalogue_proto.add_distances() = std::move(distance_proto);
  }

  return transport_catalogue_proto;
}

transport_catalogue::TransportCatalogue transport_catalogue_deserialization(
    const transport_catalogue_protobuf::TransportCatalogue
        &transport_catalogue_proto) {
  transport_catalogue::TransportCatalogue transport_catalogue;

  const auto &stops_proto = transport_catalogue_proto.stops();
  const auto &buses_proto = transport_catalogue_proto.buses();
  const auto &distances_proto = transport_catalogue_proto.distances();

  for (const auto &stop : stops_proto) {
    domain::Stop tc_stop;

    tc_stop.name = stop.name();
    tc_stop.latitude = stop.latitude();
    tc_stop.longitude = stop.longitude();

    transport_catalogue.AddStop(std::move(tc_stop));
  }

  const auto &tc_stops = transport_catalogue.GetStops();

  std::vector<domain::Distance> distances;
  for (const auto &distance : distances_proto) {
    domain::Distance tc_distance;

    tc_distance.start =
        transport_catalogue.GetStop(tc_stops[distance.start()].name);
    tc_distance.end =
        transport_catalogue.GetStop(tc_stops[distance.end()].name);

    tc_distance.distance = distance.distance();

    distances.push_back(tc_distance);
  }

  transport_catalogue.AddDistance(distances);

  for (const auto &bus_proto : buses_proto) {
    domain::Bus tc_bus;

    tc_bus.name = bus_proto.name();

    for (auto stop_id : bus_proto.stops()) {
      auto name = tc_stops[stop_id].name;
      tc_bus.stops.push_back(transport_catalogue.GetStop(name));
    }

    tc_bus.is_round_trip = bus_proto.is_round_trip();
    tc_bus.route_length = bus_proto.route_length();

    transport_catalogue.AddBus(std::move(tc_bus));
  }

  return transport_catalogue;
}

transport_catalogue_protobuf::Color color_serialization(
    const svg::Color &tc_color) {
  transport_catalogue_protobuf::Color color_proto;

  if (std::holds_alternative<std::monostate>(tc_color)) {
    color_proto.set_none(true);

  } else if (std::holds_alternative<svg::Rgb>(tc_color)) {
    svg::Rgb rgb = std::get<svg::Rgb>(tc_color);

    color_proto.mutable_rgb()->set_red(rgb.red);
    color_proto.mutable_rgb()->set_green(rgb.green);
    color_proto.mutable_rgb()->set_blue(rgb.blue);

  } else if (std::holds_alternative<svg::Rgba>(tc_color)) {
    svg::Rgba rgba = std::get<svg::Rgba>(tc_color);

    color_proto.mutable_rgba()->set_red(rgba.red);
    color_proto.mutable_rgba()->set_green(rgba.green);
    color_proto.mutable_rgba()->set_blue(rgba.blue);
    color_proto.mutable_rgba()->set_opacity(rgba.opacity);

  } else if (std::holds_alternative<std::string>(tc_color)) {
    color_proto.set_string_color(std::get<std::string>(tc_color));
  }

  return color_proto;
}

svg::Color color_deserialization(
    const transport_catalogue_protobuf::Color &color_proto) {
  svg::Color color;

  if (color_proto.has_rgb()) {
    svg::Rgb rgb;

    rgb.red = color_proto.rgb().red();
    rgb.green = color_proto.rgb().green();
    rgb.blue = color_proto.rgb().blue();

    color = rgb;

  } else if (color_proto.has_rgba()) {
    svg::Rgba rgba;

    rgba.red = color_proto.rgba().red();
    rgba.green = color_proto.rgba().green();
    rgba.blue = color_proto.rgba().blue();
    rgba.opacity = color_proto.rgba().opacity();

    color = rgba;

  } else {
    color = color_proto.string_color();
  }

  return color;
}

transport_catalogue_protobuf::RenderSettings render_settings_serialization(
    const renderer::RenderSettings &render_settings) {
  transport_catalogue_protobuf::RenderSettings render_settings_proto;

  render_settings_proto.set_width_(render_settings.width_);
  render_settings_proto.set_height_(render_settings.height_);
  render_settings_proto.set_padding_(render_settings.padding_);
  render_settings_proto.set_line_width_(render_settings.line_width_);
  render_settings_proto.set_stop_radius_(render_settings.stop_radius_);
  render_settings_proto.set_bus_label_font_size_(
      render_settings.bus_label_font_size_);

  transport_catalogue_protobuf::Point bus_label_offset_proto;
  bus_label_offset_proto.set_x(render_settings.bus_label_offset_.first);
  bus_label_offset_proto.set_y(render_settings.bus_label_offset_.second);

  *render_settings_proto.mutable_bus_label_offset_() =
      std::move(bus_label_offset_proto);

  render_settings_proto.set_stop_label_font_size_(
      render_settings.stop_label_font_size_);

  transport_catalogue_protobuf::Point stop_label_offset_proto;
  stop_label_offset_proto.set_x(render_settings.stop_label_offset_.first);
  stop_label_offset_proto.set_y(render_settings.stop_label_offset_.second);

  *render_settings_proto.mutable_stop_label_offset_() =
      std::move(stop_label_offset_proto);
  *render_settings_proto.mutable_underlayer_color_() =
      std::move(color_serialization(render_settings.underlayer_color_));
  render_settings_proto.set_underlayer_width_(
      render_settings.underlayer_width_);

  const auto &colors = render_settings.color_palette_;
  for (const auto &color : colors) {
    *render_settings_proto.add_color_palette_() =
        std::move(color_serialization(color));
  }

  return render_settings_proto;
}

renderer::RenderSettings render_settings_deserialization(
    const transport_catalogue_protobuf::RenderSettings &render_settings_proto) {
  renderer::RenderSettings render_settings;

  render_settings.width_ = render_settings_proto.width_();
  render_settings.height_ = render_settings_proto.height_();
  render_settings.padding_ = render_settings_proto.padding_();
  render_settings.line_width_ = render_settings_proto.line_width_();
  render_settings.stop_radius_ = render_settings_proto.stop_radius_();
  render_settings.bus_label_font_size_ =
      render_settings_proto.bus_label_font_size_();

  render_settings.bus_label_offset_.first =
      render_settings_proto.bus_label_offset_().x();
  render_settings.bus_label_offset_.second =
      render_settings_proto.bus_label_offset_().y();

  render_settings.stop_label_font_size_ =
      render_settings_proto.stop_label_font_size_();

  render_settings.stop_label_offset_.first =
      render_settings_proto.stop_label_offset_().x();
  render_settings.stop_label_offset_.second =
      render_settings_proto.stop_label_offset_().y();

  render_settings.underlayer_color_ =
      color_deserialization(render_settings_proto.underlayer_color_());
  render_settings.underlayer_width_ = render_settings_proto.underlayer_width_();

  for (const auto &color_proto : render_settings_proto.color_palette_()) {
    render_settings.color_palette_.push_back(
        color_deserialization(color_proto));
  }

  return render_settings;
}

transport_catalogue_protobuf::RoutingSettings routing_settings_serialization(
    const domain::RoutingSettings &routing_settings) {
  transport_catalogue_protobuf::RoutingSettings routing_settings_proto;

  routing_settings_proto.set_bus_wait_time(routing_settings.bus_wait_time);
  routing_settings_proto.set_bus_velocity(routing_settings.bus_velocity);

  return routing_settings_proto;
}

domain::RoutingSettings routing_settings_deserialization(
    const transport_catalogue_protobuf::RoutingSettings
        &routing_settings_proto) {
  domain::RoutingSettings routing_settings;

  routing_settings.bus_wait_time = routing_settings_proto.bus_wait_time();
  routing_settings.bus_velocity = routing_settings_proto.bus_velocity();

  return routing_settings;
}

void catalogue_serialization(
    const transport_catalogue::TransportCatalogue &transport_catalogue,
    const renderer::RenderSettings &render_settings,
    const domain::RoutingSettings &routing_settings, std::ostream &out) {
  transport_catalogue_protobuf::Catalogue catalogue_proto;

  transport_catalogue_protobuf::TransportCatalogue transport_catalogue_proto =
      transport_catalogue_serialization(transport_catalogue);
  transport_catalogue_protobuf::RenderSettings render_settings_proto =
      render_settings_serialization(render_settings);
  transport_catalogue_protobuf::RoutingSettings routing_settings_proto =
      routing_settings_serialization(routing_settings);

  *catalogue_proto.mutable_transport_catalogue() =
      std::move(transport_catalogue_proto);
  *catalogue_proto.mutable_render_settings() = std::move(render_settings_proto);
  *catalogue_proto.mutable_routing_settings() =
      std::move(routing_settings_proto);

  catalogue_proto.SerializePartialToOstream(&out);
}

Catalogue catalogue_deserialization(std::istream &in) {
  transport_catalogue_protobuf::Catalogue catalogue_proto;
  auto success_parsing_catalogue_from_istream =
      catalogue_proto.ParseFromIstream(&in);

  if (!success_parsing_catalogue_from_istream) {
    throw std::runtime_error("cannot parse serialized file from istream");
  }

  return {transport_catalogue_deserialization(
              catalogue_proto.transport_catalogue()),
          render_settings_deserialization(catalogue_proto.render_settings()),
          routing_settings_deserialization(catalogue_proto.routing_settings())};
}

}  // end namespace serialization
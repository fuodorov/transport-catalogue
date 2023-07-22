#include "serializer.h"

namespace serialization {

template <typename It>
uint32_t CalcId(It start, It end, std::string_view name) {
  auto stop_it = std::find_if(start, end, [&name](const domain::Stop stop) {
    return stop.name == name;
  });
  return std::distance(start, stop_it);
}

transport_catalogue_model::TransportCatalogue TransportCatalogueSerialization(
    const transport_catalogue::TransportCatalogue &transport_catalogue) {
  transport_catalogue_model::TransportCatalogue transport_catalogue_model;

  const auto &stops = transport_catalogue.GetStops();
  const auto &buses = transport_catalogue.GetBuses();
  const auto &distances = transport_catalogue.GetDistance();

  int id = 0;
  for (const auto &stop : stops) {
    transport_catalogue_model::Stop stop_model;

    stop_model.set_id(id);
    stop_model.set_name(stop.name);
    stop_model.set_latitude(stop.latitude);
    stop_model.set_longitude(stop.longitude);

    *transport_catalogue_model.add_stops() = std::move(stop_model);

    ++id;
  }

  for (const auto &bus : buses) {
    transport_catalogue_model::Bus bus_model;

    bus_model.set_name(bus.name);

    for (auto stop : bus.stops) {
      uint32_t stop_id = CalcId(stops.cbegin(), stops.cend(), stop->name);
      bus_model.add_stops(stop_id);
    }

    bus_model.set_is_round_trip(bus.is_round_trip);
    bus_model.set_route_length(bus.route_length);

    *transport_catalogue_model.add_buses() = std::move(bus_model);
  }

  for (const auto &[stops_pair, pair_distance] : distances) {
    transport_catalogue_model::Distance distance_model;

    distance_model.set_start(
        CalcId(stops.cbegin(), stops.cend(), stops_pair.first->name));

    distance_model.set_end(
        CalcId(stops.cbegin(), stops.cend(), stops_pair.second->name));

    distance_model.set_distance(pair_distance);

    *transport_catalogue_model.add_distances() = std::move(distance_model);
  }

  return transport_catalogue_model;
}

transport_catalogue::TransportCatalogue TransportCatalogueDeserialization(
    const transport_catalogue_model::TransportCatalogue
        &transport_catalogue_model) {
  transport_catalogue::TransportCatalogue transport_catalogue;

  const auto &stop_models = transport_catalogue_model.stops();
  const auto &bus_models = transport_catalogue_model.buses();
  const auto &distance_proto = transport_catalogue_model.distances();

  for (const auto &stop : stop_models) {
    domain::Stop tmp_stop;

    tmp_stop.name = stop.name();
    tmp_stop.latitude = stop.latitude();
    tmp_stop.longitude = stop.longitude();

    transport_catalogue.AddStop(std::move(tmp_stop));
  }

  const auto &tmp_stops = transport_catalogue.GetStops();

  std::vector<domain::Distance> distances;
  for (const auto &distance : distance_proto) {
    domain::Distance tmp_distance;

    tmp_distance.start =
        transport_catalogue.GetStop(tmp_stops[distance.start()].name);
    tmp_distance.end =
        transport_catalogue.GetStop(tmp_stops[distance.end()].name);

    tmp_distance.distance = distance.distance();

    distances.push_back(tmp_distance);
  }

  transport_catalogue.AddDistance(distances);

  for (const auto &bus_model : bus_models) {
    domain::Bus tmp_bus;

    tmp_bus.name = bus_model.name();

    for (auto stop_id : bus_model.stops()) {
      auto name = tmp_stops[stop_id].name;
      tmp_bus.stops.push_back(transport_catalogue.GetStop(name));
    }

    tmp_bus.is_round_trip = bus_model.is_round_trip();
    tmp_bus.route_length = bus_model.route_length();

    transport_catalogue.AddBus(std::move(tmp_bus));
  }

  return transport_catalogue;
}

transport_catalogue_model::Color ColorSerialization(
    const svg::Color &tmp_color) {
  transport_catalogue_model::Color color_model;

  if (std::holds_alternative<std::monostate>(tmp_color)) {
    color_model.set_none(true);

  } else if (std::holds_alternative<svg::Rgb>(tmp_color)) {
    svg::Rgb rgb = std::get<svg::Rgb>(tmp_color);

    color_model.mutable_rgb()->set_red(rgb.red);
    color_model.mutable_rgb()->set_green(rgb.green);
    color_model.mutable_rgb()->set_blue(rgb.blue);

  } else if (std::holds_alternative<svg::Rgba>(tmp_color)) {
    svg::Rgba rgba = std::get<svg::Rgba>(tmp_color);

    color_model.mutable_rgba()->set_red(rgba.red);
    color_model.mutable_rgba()->set_green(rgba.green);
    color_model.mutable_rgba()->set_blue(rgba.blue);
    color_model.mutable_rgba()->set_opacity(rgba.opacity);

  } else if (std::holds_alternative<std::string>(tmp_color)) {
    color_model.set_string_color(std::get<std::string>(tmp_color));
  }

  return color_model;
}

svg::Color ColorDeserialization(
    const transport_catalogue_model::Color &color_model) {
  svg::Color color;

  if (color_model.has_rgb()) {
    svg::Rgb rgb;

    rgb.red = color_model.rgb().red();
    rgb.green = color_model.rgb().green();
    rgb.blue = color_model.rgb().blue();

    color = rgb;

  } else if (color_model.has_rgba()) {
    svg::Rgba rgba;

    rgba.red = color_model.rgba().red();
    rgba.green = color_model.rgba().green();
    rgba.blue = color_model.rgba().blue();
    rgba.opacity = color_model.rgba().opacity();

    color = rgba;

  } else {
    color = color_model.string_color();
  }

  return color;
}

transport_catalogue_model::RenderSettings RenderSettingsSerialization(
    const renderer::RenderSettings &render_settings) {
  transport_catalogue_model::RenderSettings render_settings_model;

  render_settings_model.set_width_(render_settings.width_);
  render_settings_model.set_height_(render_settings.height_);
  render_settings_model.set_padding_(render_settings.padding_);
  render_settings_model.set_line_width_(render_settings.line_width_);
  render_settings_model.set_stop_radius_(render_settings.stop_radius_);
  render_settings_model.set_bus_label_font_size_(
      render_settings.bus_label_font_size_);

  transport_catalogue_model::Point bus_label_offset_proto;
  bus_label_offset_proto.set_x(render_settings.bus_label_offset_.first);
  bus_label_offset_proto.set_y(render_settings.bus_label_offset_.second);

  *render_settings_model.mutable_bus_label_offset_() =
      std::move(bus_label_offset_proto);

  render_settings_model.set_stop_label_font_size_(
      render_settings.stop_label_font_size_);

  transport_catalogue_model::Point stop_label_offset_proto;
  stop_label_offset_proto.set_x(render_settings.stop_label_offset_.first);
  stop_label_offset_proto.set_y(render_settings.stop_label_offset_.second);

  *render_settings_model.mutable_stop_label_offset_() =
      std::move(stop_label_offset_proto);
  *render_settings_model.mutable_underlayer_color_() =
      std::move(ColorSerialization(render_settings.underlayer_color_));
  render_settings_model.set_underlayer_width_(
      render_settings.underlayer_width_);

  const auto &colors = render_settings.color_palette_;
  for (const auto &color : colors) {
    *render_settings_model.add_color_palette_() =
        std::move(ColorSerialization(color));
  }

  return render_settings_model;
}

renderer::RenderSettings RenderSettingsDeserialization(
    const transport_catalogue_model::RenderSettings &render_settings_model) {
  renderer::RenderSettings render_settings;

  render_settings.width_ = render_settings_model.width_();
  render_settings.height_ = render_settings_model.height_();
  render_settings.padding_ = render_settings_model.padding_();
  render_settings.line_width_ = render_settings_model.line_width_();
  render_settings.stop_radius_ = render_settings_model.stop_radius_();
  render_settings.bus_label_font_size_ =
      render_settings_model.bus_label_font_size_();

  render_settings.bus_label_offset_.first =
      render_settings_model.bus_label_offset_().x();
  render_settings.bus_label_offset_.second =
      render_settings_model.bus_label_offset_().y();

  render_settings.stop_label_font_size_ =
      render_settings_model.stop_label_font_size_();

  render_settings.stop_label_offset_.first =
      render_settings_model.stop_label_offset_().x();
  render_settings.stop_label_offset_.second =
      render_settings_model.stop_label_offset_().y();

  render_settings.underlayer_color_ =
      ColorDeserialization(render_settings_model.underlayer_color_());
  render_settings.underlayer_width_ = render_settings_model.underlayer_width_();

  for (const auto &color_model : render_settings_model.color_palette_()) {
    render_settings.color_palette_.push_back(ColorDeserialization(color_model));
  }

  return render_settings;
}

transport_catalogue_model::RoutingSettings RoutingSettingsSerialization(
    const domain::RoutingSettings &routing_settings) {
  transport_catalogue_model::RoutingSettings routing_settings_model;

  routing_settings_model.set_bus_wait_time(routing_settings.bus_wait_time);
  routing_settings_model.set_bus_velocity(routing_settings.bus_velocity);

  return routing_settings_model;
}

domain::RoutingSettings RoutingSettingsDeserialization(
    const transport_catalogue_model::RoutingSettings &routing_settings_model) {
  domain::RoutingSettings routing_settings;

  routing_settings.bus_wait_time = routing_settings_model.bus_wait_time();
  routing_settings.bus_velocity = routing_settings_model.bus_velocity();

  return routing_settings;
}

void CatalogueSerialization(
    const transport_catalogue::TransportCatalogue &transport_catalogue,
    const renderer::RenderSettings &render_settings,
    const domain::RoutingSettings &routing_settings, std::ostream &out) {
  transport_catalogue_model::Catalogue catalogue_model;

  transport_catalogue_model::TransportCatalogue transport_catalogue_model =
      TransportCatalogueSerialization(transport_catalogue);
  transport_catalogue_model::RenderSettings render_settings_model =
      RenderSettingsSerialization(render_settings);
  transport_catalogue_model::RoutingSettings routing_settings_model =
      RoutingSettingsSerialization(routing_settings);

  *catalogue_model.mutable_transport_catalogue() =
      std::move(transport_catalogue_model);
  *catalogue_model.mutable_render_settings() = std::move(render_settings_model);
  *catalogue_model.mutable_routing_settings() =
      std::move(routing_settings_model);

  catalogue_model.SerializePartialToOstream(&out);
}

Catalogue CatalogueDeserialization(std::istream &in) {
  transport_catalogue_model::Catalogue catalogue_model;

  if (!catalogue_model.ParseFromIstream(&in)) {
    throw std::runtime_error("cannot parse serialized file from istream");
  }

  return {
      TransportCatalogueDeserialization(catalogue_model.transport_catalogue()),
      RenderSettingsDeserialization(catalogue_model.render_settings()),
      RoutingSettingsDeserialization(catalogue_model.routing_settings())};
}

}  // end namespace serialization
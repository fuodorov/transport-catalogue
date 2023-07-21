#include "renderer.h"

namespace renderer {

bool SphereProjector::is_zero(double value) {
  return std::abs(value) < EPSILON;
}

MapRenderer::MapRenderer(RenderSettings &render_settings)
    : render_settings_(render_settings) {}

svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
  return {(coords.longitude - min_lon_) * zoom_coeff_ + padding_,
          (max_lat_ - coords.latitude) * zoom_coeff_ + padding_};
}

SphereProjector MapRenderer::GetSphereProjector(
    const std::vector<geo::Coordinates> &points) const {
  return SphereProjector(points.begin(), points.end(), render_settings_.width_,
                         render_settings_.height_, render_settings_.padding_);
}

void MapRenderer::InitSphereProjector(std::vector<geo::Coordinates> points) {
  sphere_projector =
      SphereProjector(points.begin(), points.end(), render_settings_.width_,
                      render_settings_.height_, render_settings_.padding_);
}

RenderSettings MapRenderer::GetRenderSettings() const {
  return render_settings_;
}

int MapRenderer::GetPaletteSize() const {
  return render_settings_.color_palette_.size();
}

svg::Color MapRenderer::GetColor(int line_number) const {
  return render_settings_.color_palette_[line_number];
}

void MapRenderer::SetLineProperties(svg::Polyline &polyline,
                                    [[maybe_unused]] int line_number) const {
  using namespace std::literals;

  polyline.SetStrokeColor(GetColor(line_number));
  polyline.SetFillColor("none"s);
  polyline.SetStrokeWidth(render_settings_.line_width_);
  polyline.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
  polyline.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
}

void MapRenderer::SetRouteTextCommonProperties(svg::Text &text,
                                               const std::string &name,
                                               svg::Point position) const {
  using namespace std::literals;

  text.SetPosition(position);
  text.SetOffset({render_settings_.bus_label_offset_.first,
                  render_settings_.bus_label_offset_.second});
  text.SetFontSize(render_settings_.bus_label_font_size_);
  text.SetFontFamily("Verdana");
  text.SetFontWeight("bold");
  text.SetData(name);
}

void MapRenderer::SetRouteTextAdditionalProperties(svg::Text &text,
                                                   const std::string &name,
                                                   svg::Point position) const {
  SetRouteTextCommonProperties(text, name, position);

  text.SetFillColor(render_settings_.underlayer_color_);
  text.SetStrokeColor(render_settings_.underlayer_color_);
  text.SetStrokeWidth(render_settings_.underlayer_width_);
  text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
  text.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
}

void MapRenderer::SetRouteTextColorProperties(svg::Text &text,
                                              const std::string &name,
                                              int palette,
                                              svg::Point position) const {
  SetRouteTextCommonProperties(text, name, position);

  text.SetFillColor(GetColor(palette));
}

void MapRenderer::SetStopsCirclesProperties(svg::Circle &circle,
                                            svg::Point position) const {
  using namespace std::literals;

  circle.SetCenter(position);
  circle.SetRadius(render_settings_.stop_radius_);
  circle.SetFillColor("white");
}

void MapRenderer::SetStopsTextCommonProperties(svg::Text &text,
                                               const std::string &name,
                                               svg::Point position) const {
  using namespace std::literals;

  text.SetPosition(position);
  text.SetOffset({render_settings_.stop_label_offset_.first,
                  render_settings_.stop_label_offset_.second});
  text.SetFontSize(render_settings_.stop_label_font_size_);
  text.SetFontFamily("Verdana");
  text.SetData(name);
}

void MapRenderer::SetStopsTextAdditionalProperties(svg::Text &text,
                                                   const std::string &name,
                                                   svg::Point position) const {
  using namespace std::literals;
  SetStopsTextCommonProperties(text, name, position);

  text.SetFillColor(render_settings_.underlayer_color_);
  text.SetStrokeColor(render_settings_.underlayer_color_);
  text.SetStrokeWidth(render_settings_.underlayer_width_);
  text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
  text.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
}

void MapRenderer::SetStopsTextColorProperties(svg::Text &text,
                                              const std::string &name,
                                              svg::Point position) const {
  using namespace std::literals;

  SetStopsTextCommonProperties(text, name, position);
  text.SetFillColor("black");
}

void MapRenderer::AddLine(std::vector<std::pair<Bus *, int>> &buses_palette) {
  std::vector<geo::Coordinates> stops_geo_coords;

  for (auto [bus, palette] : buses_palette) {
    for (Stop *stop : bus->stops) {
      geo::Coordinates coordinates;
      coordinates.latitude = stop->latitude;
      coordinates.longitude = stop->longitude;

      stops_geo_coords.push_back(coordinates);
    }

    svg::Polyline bus_line;
    bool bus_empty = true;

    for (auto &coord : stops_geo_coords) {
      bus_empty = false;
      bus_line.AddPoint(sphere_projector(coord));
    }

    if (!bus_empty) {
      SetLineProperties(bus_line, palette);
      map_svg.Add(bus_line);
    }

    stops_geo_coords.clear();
  }
}

void MapRenderer::AddBusesName(
    std::vector<std::pair<Bus *, int>> &buses_palette) {
  std::vector<geo::Coordinates> stops_geo_coords;
  bool bus_empty = true;

  for (auto [bus, palette] : buses_palette) {
    for (Stop *stop : bus->stops) {
      geo::Coordinates coordinates;
      coordinates.latitude = stop->latitude;
      coordinates.longitude = stop->longitude;

      stops_geo_coords.push_back(coordinates);

      if (bus_empty) bus_empty = false;
    }

    svg::Text route_name_roundtrip;
    svg::Text route_title_roundtrip;
    svg::Text route_name_notroundtrip;
    svg::Text route_title_notroundtrip;

    if (!bus_empty) {
      if (bus->is_round_trip) {
        SetRouteTextAdditionalProperties(route_name_roundtrip,
                                         std::string(bus->name),
                                         sphere_projector(stops_geo_coords[0]));
        map_svg.Add(route_name_roundtrip);

        SetRouteTextColorProperties(route_title_roundtrip,
                                    std::string(bus->name), palette,
                                    sphere_projector(stops_geo_coords[0]));
        map_svg.Add(route_title_roundtrip);

      } else {
        SetRouteTextAdditionalProperties(route_name_roundtrip,
                                         std::string(bus->name),
                                         sphere_projector(stops_geo_coords[0]));
        map_svg.Add(route_name_roundtrip);

        SetRouteTextColorProperties(route_title_roundtrip,
                                    std::string(bus->name), palette,
                                    sphere_projector(stops_geo_coords[0]));
        map_svg.Add(route_title_roundtrip);

        if (stops_geo_coords[0] !=
            stops_geo_coords[stops_geo_coords.size() / 2]) {
          SetRouteTextAdditionalProperties(
              route_name_notroundtrip, std::string(bus->name),
              sphere_projector(stops_geo_coords[stops_geo_coords.size() / 2]));
          map_svg.Add(route_name_notroundtrip);

          SetRouteTextColorProperties(
              route_title_notroundtrip, std::string(bus->name), palette,
              sphere_projector(stops_geo_coords[stops_geo_coords.size() / 2]));
          map_svg.Add(route_title_notroundtrip);
        }
      }
    }

    bus_empty = false;
    stops_geo_coords.clear();
  }
}

void MapRenderer::AddStopsCircle(std::vector<Stop *> &stops) {
  std::vector<geo::Coordinates> stops_geo_coords;
  svg::Circle icon;

  for (Stop *stop_info : stops) {
    if (stop_info) {
      geo::Coordinates coordinates;
      coordinates.latitude = stop_info->latitude;
      coordinates.longitude = stop_info->longitude;

      SetStopsCirclesProperties(icon, sphere_projector(coordinates));
      map_svg.Add(icon);
    }
  }
}

void MapRenderer::AddStopsName(std::vector<Stop *> &stops) {
  std::vector<geo::Coordinates> stops_geo_coords;

  svg::Text svg_stop_name;
  svg::Text svg_stop_name_title;

  for (Stop *stop_info : stops) {
    if (stop_info) {
      geo::Coordinates coordinates;
      coordinates.latitude = stop_info->latitude;
      coordinates.longitude = stop_info->longitude;

      SetStopsTextAdditionalProperties(svg_stop_name, stop_info->name,
                                       sphere_projector(coordinates));
      map_svg.Add(svg_stop_name);

      SetStopsTextColorProperties(svg_stop_name_title, stop_info->name,
                                  sphere_projector(coordinates));
      map_svg.Add(svg_stop_name_title);
    }
  }
}

void MapRenderer::GetStreamMap(std::ostream &stream_) {
  map_svg.Render(stream_);
}

}  // end namespace renderer
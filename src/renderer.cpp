#include "renderer.h"

namespace renderer {

bool SphereProjector::IsLessEpsilon(double value) {
  return std::abs(value) < EPSILON;
}

MapRenderer::MapRenderer(RenderSettings &render_settings)
    : render_settings_(render_settings) {}

svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
  return {(coords.longitude - min_longitude_) * zoom_coeffIcient_ + padding_,
          (max_latitude_ - coords.latitude) * zoom_coeffIcient_ + padding_};
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

  polyline.SetStrokeColor(GetColor(line_number))
      .SetFillColor(NONE_FILL_COLOR)
      .SetStrokeWidth(render_settings_.line_width_)
      .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
      .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
}

void MapRenderer::SetRouteTextCommonProperties(svg::Text &text,
                                               const std::string &name,
                                               svg::Point position) const {
  using namespace std::literals;

  text.SetPosition(position)
      .SetOffset({render_settings_.bus_label_offset_.first,
                  render_settings_.bus_label_offset_.second})
      .SetFontSize(render_settings_.bus_label_font_size_)
      .SetFontFamily(VERDANA_FONT_FAMILY)
      .SetFontWeight(BOLD_FONT_WEIGHT)
      .SetData(name);
}

void MapRenderer::SetRouteTextAdditionalProperties(svg::Text &text,
                                                   const std::string &name,
                                                   svg::Point position) const {
  SetRouteTextCommonProperties(text, name, position);

  text.SetFillColor(render_settings_.underlayer_color_)
      .SetStrokeColor(render_settings_.underlayer_color_)
      .SetStrokeWidth(render_settings_.underlayer_width_)
      .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
      .SetStrokeLineCap(svg::StrokeLineCap::ROUND);
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

  circle.SetCenter(position)
      .SetRadius(render_settings_.stop_radius_)
      .SetFillColor(WHITE_FILL_COLOR);
}

void MapRenderer::SetStopsTextCommonProperties(svg::Text &text,
                                               const std::string &name,
                                               svg::Point position) const {
  using namespace std::literals;

  text.SetPosition(position)
      .SetOffset({render_settings_.stop_label_offset_.first,
                  render_settings_.stop_label_offset_.second})
      .SetFontSize(render_settings_.stop_label_font_size_)
      .SetFontFamily(VERDANA_FONT_FAMILY)
      .SetData(name);
}

void MapRenderer::SetStopsTextAdditionalProperties(svg::Text &text,
                                                   const std::string &name,
                                                   svg::Point position) const {
  using namespace std::literals;
  SetStopsTextCommonProperties(text, name, position);

  text.SetFillColor(render_settings_.underlayer_color_)
      .SetStrokeColor(render_settings_.underlayer_color_)
      .SetStrokeWidth(render_settings_.underlayer_width_)
      .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
      .SetStrokeLineCap(svg::StrokeLineCap::ROUND);
}

void MapRenderer::SetStopsTextColorProperties(svg::Text &text,
                                              const std::string &name,
                                              svg::Point position) const {
  using namespace std::literals;

  SetStopsTextCommonProperties(text, name, position);
  text.SetFillColor(BLACK_FILL_COLOR);
}

void MapRenderer::AddLine(std::vector<std::pair<Bus *, int>> &palettes) {
  std::vector<geo::Coordinates> coordinates;

  for (auto [bus, palette] : palettes) {
    for (Stop *stop : bus->stops) {
      coordinates.push_back({stop->latitude, stop->longitude});
    }

    svg::Polyline line;
    bool is_empty = true;

    for (auto &coord : coordinates) {
      is_empty = false;
      line.AddPoint(sphere_projector(coord));
    }

    if (!is_empty) {
      SetLineProperties(line, palette);
      map_svg.Add(line);
    }

    coordinates.clear();
  }
}

void MapRenderer::AddBusesName(std::vector<std::pair<Bus *, int>> &palettes) {
  std::vector<geo::Coordinates> coordinates;
  bool is_empty = true;

  for (auto [bus, palette] : palettes) {
    for (Stop *stop : bus->stops) {
      coordinates.push_back({stop->latitude, stop->longitude});
      if (is_empty) is_empty = false;
    }

    svg::Text name_is_round_trip, title_is_round_trip, name_is_not_round_trip,
        title_is_not_round_trip;

    if (!is_empty) {
      if (bus->is_round_trip) {
        SetRouteTextAdditionalProperties(name_is_round_trip,
                                         std::string(bus->name),
                                         sphere_projector(coordinates[0]));
        map_svg.Add(name_is_round_trip);

        SetRouteTextColorProperties(title_is_round_trip, std::string(bus->name),
                                    palette, sphere_projector(coordinates[0]));
        map_svg.Add(title_is_round_trip);

      } else {
        SetRouteTextAdditionalProperties(name_is_round_trip,
                                         std::string(bus->name),
                                         sphere_projector(coordinates[0]));
        map_svg.Add(name_is_round_trip);

        SetRouteTextColorProperties(title_is_round_trip, std::string(bus->name),
                                    palette, sphere_projector(coordinates[0]));
        map_svg.Add(title_is_round_trip);

        if (coordinates[0] != coordinates[coordinates.size() / 2]) {
          SetRouteTextAdditionalProperties(
              name_is_not_round_trip, std::string(bus->name),
              sphere_projector(coordinates[coordinates.size() / 2]));
          map_svg.Add(name_is_not_round_trip);

          SetRouteTextColorProperties(
              title_is_not_round_trip, std::string(bus->name), palette,
              sphere_projector(coordinates[coordinates.size() / 2]));
          map_svg.Add(title_is_not_round_trip);
        }
      }
    }

    is_empty = false;
    coordinates.clear();
  }
}

void MapRenderer::AddStopsCircle(std::vector<Stop *> &stops) {
  std::vector<geo::Coordinates> coordinates;
  svg::Circle icon;

  for (Stop *stop : stops) {
    if (stop) {
      SetStopsCirclesProperties(
          icon, sphere_projector({stop->latitude, stop->longitude}));
      map_svg.Add(icon);
    }
  }
}

void MapRenderer::AddStopsName(std::vector<Stop *> &stops) {
  std::vector<geo::Coordinates> coordinates;
  svg::Text name, title;

  for (Stop *stop : stops) {
    if (stop) {
      SetStopsTextAdditionalProperties(
          name, stop->name,
          sphere_projector({stop->latitude, stop->longitude}));

      map_svg.Add(name);

      SetStopsTextColorProperties(
          title, stop->name,
          sphere_projector({stop->latitude, stop->longitude}));
      map_svg.Add(title);
    }
  }
}

void MapRenderer::GetStreamMap(std::ostream &stream_) {
  map_svg.Render(stream_);
}

}  // end namespace renderer
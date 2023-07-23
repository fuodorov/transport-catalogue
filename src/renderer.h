#pragma once
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>

#include "domain.h"
#include "geo/geo.h"
#include "svg/svg.h"

using namespace domain;

namespace renderer {

inline const double EPSILON = 1e-6;
inline const char *VERDANA_FONT_FAMILY = "Verdana";
inline const char *BLACK_FILL_COLOR = "black";
inline const char *WHITE_FILL_COLOR = "white";
inline const char *NONE_FILL_COLOR = "none";
inline const char *BOLD_FONT_WEIGHT = "bold";

class SphereProjector {
 public:
  SphereProjector() = default;

  template <typename InputIt>
  SphereProjector(InputIt points_begin, InputIt points_end, double max_width,
                  double max_height, double padding);

  svg::Point operator()(geo::Coordinates coords) const;

 private:
  double padding_;
  double min_longitude_ = 0;
  double max_latitude_ = 0;
  double zoom_coeffIcient_ = 0;

  bool IsLessEpsilon(double value);
};

struct RenderSettings {
  double width_;
  double height_;
  double padding_;
  double line_width_;
  double stop_radius_;
  int bus_label_font_size_;
  std::pair<double, double> bus_label_offset_;
  int stop_label_font_size_;
  std::pair<double, double> stop_label_offset_;
  svg::Color underlayer_color_;
  double underlayer_width_;
  std::vector<svg::Color> color_palette_;
};

class MapRenderer {
 public:
  MapRenderer(RenderSettings &render_settings);

  SphereProjector GetSphereProjector(
      const std::vector<geo::Coordinates> &points) const;
  void InitSphereProjector(std::vector<geo::Coordinates> points);

  RenderSettings GetRenderSettings() const;
  int GetPaletteSize() const;
  svg::Color GetColor(int line_number) const;

  void SetLineProperties(svg::Polyline &polyline, int line_number) const;

  void SetRouteTextCommonProperties(svg::Text &text, const std::string &name,
                                    svg::Point position) const;
  void SetRouteTextAdditionalProperties(svg::Text &text,
                                        const std::string &name,
                                        svg::Point position) const;
  void SetRouteTextColorProperties(svg::Text &text, const std::string &name,
                                   int palette, svg::Point position) const;

  void SetStopsCirclesProperties(svg::Circle &circle,
                                 svg::Point position) const;

  void SetStopsTextCommonProperties(svg::Text &text, const std::string &name,
                                    svg::Point position) const;
  void SetStopsTextAdditionalProperties(svg::Text &text,
                                        const std::string &name,
                                        svg::Point position) const;
  void SetStopsTextColorProperties(svg::Text &text, const std::string &name,
                                   svg::Point position) const;

  void AddLine(std::vector<std::pair<Bus *, int>> &palettes);
  void AddBusesName(std::vector<std::pair<Bus *, int>> &palettes);
  void AddStopsCircle(std::vector<Stop *> &stops_name);
  void AddStopsName(std::vector<Stop *> &stops_name);

  void GetStreamMap(std::ostream &stream_);

 private:
  SphereProjector sphere_projector;
  RenderSettings &render_settings_;
  svg::Document map_svg;
};

template <typename InputIt>
SphereProjector::SphereProjector(InputIt points_begin, InputIt points_end,
                                 double max_width, double max_height,
                                 double padding)
    : padding_(padding) {
  if (points_begin == points_end) {
    return;
  }

  const auto [left_it, right_it] = std::minmax_element(
      points_begin, points_end,
      [](auto lhs, auto rhs) { return lhs.longitude < rhs.longitude; });

  min_longitude_ = left_it->longitude;
  const double max_lon = right_it->longitude;

  const auto [bottom_it, top_it] = std::minmax_element(
      points_begin, points_end,
      [](auto lhs, auto rhs) { return lhs.latitude < rhs.latitude; });

  const double min_lat = bottom_it->latitude;
  max_latitude_ = top_it->latitude;

  std::optional<double> width_zoom;
  if (!IsLessEpsilon(max_lon - min_longitude_)) {
    width_zoom = (max_width - 2 * padding) / (max_lon - min_longitude_);
  }

  std::optional<double> height_zoom;
  if (!IsLessEpsilon(max_latitude_ - min_lat)) {
    height_zoom = (max_height - 2 * padding) / (max_latitude_ - min_lat);
  }

  if (width_zoom && height_zoom) {
    zoom_coeffIcient_ = std::min(*width_zoom, *height_zoom);
  } else if (width_zoom) {
    zoom_coeffIcient_ = *width_zoom;

  } else if (height_zoom) {
    zoom_coeffIcient_ = *height_zoom;
  }
}

}  // end namespace renderer
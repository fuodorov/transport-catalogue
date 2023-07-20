#pragma once
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>

#include "domain.h"
#include "geo/geo.h"
#include "svg/svg.h"

using namespace domain;

namespace map_renderer {

inline const double EPSILON = 1e-6;

class SphereProjector {
public:
    SphereProjector() = default;

    template <typename InputIt>
    SphereProjector(InputIt points_begin, InputIt points_end, double max_width, double max_height, double padding);

    svg::Point operator()(geo::Coordinates coords) const;

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;

    bool is_zero(double value);
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

    SphereProjector get_sphere_projector(const std::vector<geo::Coordinates> &points) const;
    void init_sphere_projector(std::vector<geo::Coordinates> points);

    RenderSettings get_render_settings() const;
    int get_palette_size() const;
    svg::Color get_color(int line_number) const;

    void set_line_properties(svg::Polyline &polyline, int line_number) const;

    void set_route_text_common_properties(svg::Text &text, const std::string &name, svg::Point position) const;
    void set_route_text_additional_properties(svg::Text &text, const std::string &name, svg::Point position) const;
    void set_route_text_color_properties(svg::Text &text, const std::string &name, int palette,
                                         svg::Point position) const;

    void set_stops_circles_properties(svg::Circle &circle, svg::Point position) const;

    void set_stops_text_common_properties(svg::Text &text, const std::string &name, svg::Point position) const;
    void set_stops_text_additional_properties(svg::Text &text, const std::string &name, svg::Point position) const;
    void set_stops_text_color_properties(svg::Text &text, const std::string &name, svg::Point position) const;

    void add_line(std::vector<std::pair<Bus *, int>> &buses_palette);
    void add_buses_name(std::vector<std::pair<Bus *, int>> &buses_palette);
    void add_stops_circle(std::vector<Stop *> &stops_name);
    void add_stops_name(std::vector<Stop *> &stops_name);

    void get_stream_map(std::ostream &stream_);

private:
    SphereProjector sphere_projector;
    RenderSettings &render_settings_;
    svg::Document map_svg;
};

template <typename InputIt>
SphereProjector::SphereProjector(InputIt points_begin, InputIt points_end, double max_width, double max_height,
                                 double padding)
    : padding_(padding) {
    if (points_begin == points_end) {
        return;
    }

    const auto [left_it, right_it] =
        std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) { return lhs.longitude < rhs.longitude; });

    min_lon_ = left_it->longitude;
    const double max_lon = right_it->longitude;

    const auto [bottom_it, top_it] =
        std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) { return lhs.latitude < rhs.latitude; });

    const double min_lat = bottom_it->latitude;
    max_lat_ = top_it->latitude;

    std::optional<double> width_zoom;
    if (!is_zero(max_lon - min_lon_)) {
        width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
    }

    std::optional<double> height_zoom;
    if (!is_zero(max_lat_ - min_lat)) {
        height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
    }

    if (width_zoom && height_zoom) {
        zoom_coeff_ = std::min(*width_zoom, *height_zoom);
    } else if (width_zoom) {
        zoom_coeff_ = *width_zoom;

    } else if (height_zoom) {
        zoom_coeff_ = *height_zoom;
    }
}

}  // end namespace map_renderer
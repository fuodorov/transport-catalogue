#include "renderer.h"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace renderer {
    using namespace std::literals;

    Visualization& Visualization::SetScreen(const Screen& screen) {
        screen_ = screen;
        return *this;
    }
    Visualization& Visualization::SetLineWidth(double width) {
        line_width_ = width;
        return *this;
    }
    Visualization& Visualization::SetStopRadius(double radius) {
        stop_radius_ = radius;
        return *this;
    }

    Visualization& Visualization::SetLabels(LabelType type, Label label) {
        labels_.emplace(type, label);
        return *this;
    }

    Visualization& Visualization::SetUnderLayer(UnderLayer layer) {
        under_layer_ = std::move(layer);
        return *this;
    }

    Visualization& Visualization::SetColors(std::vector<svg::Color> colors) {
        colors_ = std::move(colors);
        return *this;
    }

    MapImageRenderer::MapImageRenderer(const catalogue::TransportCatalogue& catalogue, const Visualization& settings, svg::Document& image): catalogue_(catalogue),
        settings_(settings),
        image_(image),
        min_lng_(catalogue_.GetMinStopCoordinates().lng),
        max_lat_(catalogue_.GetMaxStopCoordinates().lat),
        zoom_(CalculateZoom()) {}

    void MapImageRenderer::Render() {
        PutRouteLines();
        PutRouteNames();
        PutStopCircles();
        PutStopNames();
    }

    void MapImageRenderer::PutRouteLines() {
        const double& width = settings_.line_width_;

        int route_id{0};
        bool is_previous_route_empty{true};

        for (std::string_view bus_name : catalogue_.GetOrderedBusList()) {
            auto [bus, stops] = catalogue_.GetRouteInfo(bus_name);

            route_id = is_previous_route_empty ? route_id : route_id + 1;

            svg::Polyline route;
            for (const auto& stop : stops)
                route.AddPoint(ToScreenPosition(stop->point));

            image_.Add(route.SetStrokeColor(TakeColorById(route_id))
                        .SetFillColor("none"s)
                        .SetStrokeWidth(width)
                        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));

            is_previous_route_empty = stops.empty();
        }
    }

    void MapImageRenderer::PutRouteNames() {
        const auto& bus_settings = settings_.labels_.at(LabelType::Bus);
        const auto& under_layer_settings = settings_.under_layer_;

        int route_id{0};
        bool is_previous_route_empty{true};

        for (std::string_view bus_name : catalogue_.GetOrderedBusList()) {
            auto [bus, stops] = catalogue_.GetFinalStops(bus_name);

            route_id = is_previous_route_empty ? route_id : route_id + 1;

            if (stops.empty())
                continue;

            for (const auto& stop : stops) {
                image_.Add(svg::Text()
                            .SetData(bus->number)
                            .SetFillColor(under_layer_settings.color_)
                            .SetStrokeColor(under_layer_settings.color_)
                            .SetStrokeWidth(under_layer_settings.width_)
                            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                            .SetPosition(ToScreenPosition(stop->point))
                            .SetOffset(bus_settings.offset_)
                            .SetFontSize(bus_settings.font_size_)
                            .SetFontFamily(constants::DEFAULT_FONT_FAMILY)
                            .SetFontWeight(constants::DEFAULT_FONT_WEIGHT));

                image_.Add(svg::Text()
                            .SetData(bus->number)
                            .SetPosition(ToScreenPosition(stop->point))
                            .SetOffset(bus_settings.offset_)
                            .SetFontSize(bus_settings.font_size_)
                            .SetFontFamily(constants::DEFAULT_FONT_FAMILY)
                            .SetFontWeight(constants::DEFAULT_FONT_WEIGHT)
                            .SetFillColor(TakeColorById(route_id)));
            }

            is_previous_route_empty = stops.empty();
        }
    }

    void MapImageRenderer::PutStopCircles() {
        for (const auto& [_, stop] : catalogue_.GetAllStopsFromRoutes())
            image_.Add(svg::Circle()
                        .SetCenter(ToScreenPosition(stop->point))
                        .SetRadius(settings_.stop_radius_)
                        .SetFillColor("white"s));
    }

    void MapImageRenderer::PutStopNames() {
        const auto& stop_settings = settings_.labels_.at(LabelType::Stop);
        const auto& under_layer_settings = settings_.under_layer_;

        for (const auto& [_, stop] : catalogue_.GetAllStopsFromRoutes()) {
            image_.Add(svg::Text()
                        .SetData(stop->name)
                        .SetFillColor(under_layer_settings.color_)
                        .SetStrokeColor(under_layer_settings.color_)
                        .SetStrokeWidth(under_layer_settings.width_)
                        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                        .SetPosition(ToScreenPosition(stop->point))
                        .SetOffset(stop_settings.offset_)
                        .SetFontSize(stop_settings.font_size_)
                        .SetFontFamily(constants::DEFAULT_FONT_FAMILY));

            image_.Add(svg::Text()
                        .SetData(stop->name)
                        .SetFillColor("black"s)
                        .SetPosition(ToScreenPosition(stop->point))
                        .SetOffset(stop_settings.offset_)
                        .SetFontSize(stop_settings.font_size_)
                        .SetFontFamily(constants::DEFAULT_FONT_FAMILY));
        }
    }

    double MapImageRenderer::CalculateZoom() const {
        double zoom{0.};
        const auto [min_lat, min_lng] = catalogue_.GetMinStopCoordinates();
        const auto [max_lat, max_lng] = catalogue_.GetMaxStopCoordinates();

        auto make_zoom_coefficient = [&](double max_min_diff, double screen_size) -> double {
            return (max_min_diff == 0.) ? std::numeric_limits<double>::max() : (screen_size - 2. * settings_.screen_.padding_) / max_min_diff;
        };

        double zoom_x = make_zoom_coefficient(max_lng - min_lng, settings_.screen_.width_);
        double zoom_y = make_zoom_coefficient(max_lat - min_lat, settings_.screen_.height_);

        zoom = std::min(zoom_x, zoom_y);
        return (zoom == std::numeric_limits<double>::max()) ? 0. : zoom;
    }

    svg::Color MapImageRenderer::TakeColorById(int route_id) const {
        return settings_.colors_.at(static_cast<int>(route_id) % settings_.colors_.size());
    }

    svg::Point MapImageRenderer::ToScreenPosition(geo::Coordinates position) {
        svg::Point point;
        point.x = (position.lng - min_lng_) * zoom_ + settings_.screen_.padding_;
        point.y = (max_lat_ - position.lat) * zoom_ + settings_.screen_.padding_;
        return point;
    }

    std::string RenderTransportMap(const catalogue::TransportCatalogue& catalogue, const Visualization& settings) {
        svg::Document image;
        MapImageRenderer renderer{catalogue, settings, image};
        renderer.Render();
        std::stringstream ss;
        image.Render(ss);
        return ss.str();
    }

}  // namespace render
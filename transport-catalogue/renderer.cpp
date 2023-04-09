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
        int id{0};
        bool is_empty{true};

        for (std::string_view bus_name : catalogue_.GetOrderedBusList()) {
            auto [bus, stops] = catalogue_.GetRouteInfo(bus_name);
            id = is_empty ? id : id + 1;
            
            svg::Polyline route;
            for (const auto& stop : stops){
                route.AddPoint(ToScreenPosition(stop->point));
            }

            image_.Add(route.SetStrokeColor(TakeColorById(id))
                        .SetFillColor("none"s)
                        .SetStrokeWidth(settings_.line_width_)
                        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));

            is_empty = stops.empty();
        }
    }

    void MapImageRenderer::PutRouteNames() {
        int id{0};
        bool is_empty{true};

        for (std::string_view bus_name : catalogue_.GetOrderedBusList()) {
            auto [bus, stops] = catalogue_.GetFinalStops(bus_name);
            id = is_empty ? id : id + 1;
            
            if (stops.empty()){
                continue;
            }

            for (const auto& stop : stops) {
                image_.Add(svg::Text()
                            .SetData(bus->number)
                            .SetFillColor(settings_.under_layer_.color_)
                            .SetStrokeColor(settings_.under_layer_.color_)
                            .SetStrokeWidth(settings_.under_layer_.width_)
                            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                            .SetPosition(ToScreenPosition(stop->point))
                            .SetOffset(settings_.labels_.at(LabelType::Bus).offset_)
                            .SetFontSize(settings_.labels_.at(LabelType::Bus).font_size_)
                            .SetFontFamily(constants::DEFAULT_FONT_FAMILY)
                            .SetFontWeight(constants::DEFAULT_FONT_WEIGHT));

                image_.Add(svg::Text()
                            .SetData(bus->number)
                            .SetPosition(ToScreenPosition(stop->point))
                            .SetOffset(settings_.labels_.at(LabelType::Bus).offset_)
                            .SetFontSize(settings_.labels_.at(LabelType::Bus).font_size_)
                            .SetFontFamily(constants::DEFAULT_FONT_FAMILY)
                            .SetFontWeight(constants::DEFAULT_FONT_WEIGHT)
                            .SetFillColor(TakeColorById(id)));
            }

            is_empty = stops.empty();
        }
    }

    void MapImageRenderer::PutStopCircles() {
        for (const auto& [_, stop] : catalogue_.GetAllStopsFromRoutes()){
            image_.Add(svg::Circle()
                        .SetCenter(ToScreenPosition(stop->point))
                        .SetRadius(settings_.stop_radius_)
                        .SetFillColor("white"s));
        }
    }

    void MapImageRenderer::PutStopNames() {
        for (const auto& [_, stop] : catalogue_.GetAllStopsFromRoutes()) {
            image_.Add(svg::Text()
                        .SetData(stop->name)
                        .SetFillColor(settings_.under_layer_.color_)
                        .SetStrokeColor(settings_.under_layer_.color_)
                        .SetStrokeWidth(settings_.under_layer_.width_)
                        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                        .SetPosition(ToScreenPosition(stop->point))
                        .SetOffset(settings_.labels_.at(LabelType::Stop).offset_)
                        .SetFontSize(settings_.labels_.at(LabelType::Stop).font_size_)
                        .SetFontFamily(constants::DEFAULT_FONT_FAMILY));

            image_.Add(svg::Text()
                        .SetData(stop->name)
                        .SetFillColor("black"s)
                        .SetPosition(ToScreenPosition(stop->point))
                        .SetOffset(settings_.labels_.at(LabelType::Stop).offset_)
                        .SetFontSize(settings_.labels_.at(LabelType::Stop).font_size_)
                        .SetFontFamily(constants::DEFAULT_FONT_FAMILY));
        }
    }

    double MapImageRenderer::CalculateZoom() const {
        double zoom{0.};
        const auto [min_lat, min_lng] = catalogue_.GetMinStopCoordinates();
        const auto [max_lat, max_lng] = catalogue_.GetMaxStopCoordinates();

        auto get_zoom = [&](double diff, double screen_size) -> double {
            return (diff == 0.) ? std::numeric_limits<double>::max() : (screen_size - 2. * settings_.screen_.padding_) / diff;
        };

        double zoom_x = get_zoom(max_lng - min_lng, settings_.screen_.width_);
        double zoom_y = get_zoom(max_lat - min_lat, settings_.screen_.height_);

        zoom = std::min(zoom_x, zoom_y);
        return (zoom == std::numeric_limits<double>::max()) ? 0. : zoom;
    }

    svg::Color MapImageRenderer::TakeColorById(int id) const {
        return settings_.colors_.at(static_cast<int>(id) % settings_.colors_.size());
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

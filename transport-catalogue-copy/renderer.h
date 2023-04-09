#pragma once

#include "svg.h"
#include "catalogue.h"

namespace renderer {
    enum class LabelType { Bus = 0, Stop = 1 };

    struct Screen {
        double width_{0.}, height_{0.}, padding_{0.};
    };

    struct UnderLayer {
        svg::Color color_;
        double width_;
    };

    struct Label {
        int font_size_{0};
        svg::Point offset_;
    };

    class Visualization {
        friend class MapImageRenderer;

    public:
        Visualization() = default;

        Visualization& SetScreen(const Screen& screen);
        Visualization& SetLineWidth(double width);
        Visualization& SetStopRadius(double radius);

        Visualization& SetLabels(LabelType type, Label label);
        Visualization& SetUnderLayer(UnderLayer layer);
        Visualization& SetColors(std::vector<svg::Color> colors);

    private:
        Screen screen_;
        double line_width_{0.}, stop_radius_{0.};

        std::unordered_map<LabelType, Label> labels_;
        UnderLayer under_layer_;
        std::vector<svg::Color> colors_;
    };

    class MapImageRenderer {
    public:
        MapImageRenderer(const catalogue::TransportCatalogue& catalogue, const Visualization& settings, svg::Document& image);

        void Render();

    private:
        void PutRouteLines();
        void PutRouteNames();
        void PutStopCircles();
        void PutStopNames();

        [[nodiscard]] double CalculateZoom() const;
        [[nodiscard]] svg::Color TakeColorById(int route_id) const;
        svg::Point ToScreenPosition(geo::Coordinates position);

        const catalogue::TransportCatalogue& catalogue_;
        const Visualization& settings_;
        svg::Document& image_;

        double min_lng_{0.};
        double max_lat_{0.};
        double zoom_{0.};
    };

    std::string RenderTransportMap(const catalogue::TransportCatalogue& catalogue, const Visualization& settings);

}  // namespace render

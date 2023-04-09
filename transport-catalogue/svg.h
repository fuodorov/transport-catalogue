#pragma once

#include <any>
#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

namespace svg {
    struct Point {
        Point() = default;
        Point(double x, double y) : x(x), y(y) {}
        double x{0.}, y{0.};
    };

    struct RenderContext {
        public:
            explicit RenderContext(std::ostream& out) : out(out) {}
            RenderContext(std::ostream& out, int indent_step, int indent = 0) : out(out), indent_step(indent_step), indent(indent) {}
            [[nodiscard]] RenderContext Indented() const;
            void RenderIndent() const;
            std::ostream& out;
            int indent_step{0}, indent{0};
    };

    struct Rgb {
        uint8_t red{0}, green{0}, blue{0};
        Rgb() = default;
        Rgb(uint8_t red, uint8_t green, uint8_t blue) : red(red), green(green), blue(blue) {}
    };

    struct Rgba : public Rgb {
        double opacity{1.};
        Rgba() = default;
        Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity) : Rgb(red, green, blue), opacity(opacity) {}
    };

    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

    inline const Color NoneColor{"none"};

    struct ColorPrinter {
        std::ostream& os;
        void operator()(std::monostate) const;
        void operator()(const std::string& color) const;
        void operator()(Rgb color) const;
        void operator()(Rgba color) const;
    };

    std::ostream& operator<<(std::ostream& os, const Color& color);

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    std::ostream& operator<<(std::ostream& os, const StrokeLineCap& value);

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    std::ostream& operator<<(std::ostream& os, const StrokeLineJoin& value);

    class Object {
        public:
            void Render(const RenderContext& context) const;
            void Render(std::ostream& os) const;
            virtual ~Object() = default;

        private:
            virtual void RenderObject(const RenderContext& context) const = 0;
    };

    template <class ObjectType>
    class PathProps {
        public:
            ObjectType& SetFillColor(Color color) {
                color_ = color;
                return AsObjectType();
            }
            ObjectType& SetStrokeColor(Color stroke_color) {
                stroke_color_ = stroke_color;
                return AsObjectType();
            }
            ObjectType& SetStrokeWidth(double width) {
                stroke_width_ = width;
                return AsObjectType();
            }
            ObjectType& SetStrokeLineCap(StrokeLineCap line_cap) {
                line_cap_ = line_cap;
                return AsObjectType();
            }
            ObjectType& SetStrokeLineJoin(StrokeLineJoin line_join) {
                line_join_ = line_join;
                return AsObjectType();
            }

        protected:
            ~PathProps() = default;

            void RenderAttrs(std::ostream& os) const {
                using namespace std::literals;
                PrintProperty(os, "fill"sv, color_);
                PrintProperty(os, "stroke"sv, stroke_color_);
                PrintProperty(os, "stroke-width"sv, stroke_width_);
                PrintProperty(os, "stroke-linecap"sv, line_cap_);
                PrintProperty(os, "stroke-linejoin"sv, line_join_);
            }

            void RenderAttrs(const RenderContext& context) const {
                RenderContext(context.out);
            }

            std::optional<Color> color_, stroke_color_;
            std::optional<double> stroke_width_;
            std::optional<StrokeLineCap> line_cap_;
            std::optional<StrokeLineJoin> line_join_;

        private:
            ObjectType& AsObjectType() {
                return static_cast<ObjectType&>(*this);
            }

            template <class PropertyType>
            void PrintProperty(std::ostream& os, std::string_view tag_name,
                            const std::optional<PropertyType>& tag_value) const {
                if (tag_value){
                    os << " " << tag_name << "=\"" << *tag_value << "\"";
                }
            }
    };

    class Circle final : public Object, public PathProps<Circle> {
        public:
            Circle& SetCenter(Point center);
            Circle& SetRadius(double radius);

        private:
            void RenderObject(const RenderContext& context) const override;
            Point center_;
            double radius_ = 1.0;
    };

    class Polyline final : public Object, public PathProps<Polyline> {
        public:
            Polyline& AddPoint(Point point);

        private:
            void RenderObject(const RenderContext& context) const override;
            std::vector<Point> vertexes_;
    };

    class Text final : public Object, public PathProps<Text> {
        public:
            struct EscapeCharacter {
                char character;
                std::string replacement;
            };

            Text& SetPosition(Point position = Point());
            Text& SetOffset(Point offset = Point());
            Text& SetFontSize(uint32_t size);
            Text& SetFontFamily(std::string font_family);
            Text& SetFontWeight(std::string font_weight);
            Text& SetData(std::string data);

        private:
            void RenderObject(const RenderContext& context) const override;
            [[nodiscard]] std::string PreprocessTest(const std::string& input_text) const;
            inline static const std::vector<EscapeCharacter> kEscapeCharacters{{'&', "&amp;"}, {'"', "&quot;"}, {'\'', "&apos;"}, {'<', "&lt;"}, {'>', "&gt;"}};
            Point position_, offset_;
            uint32_t font_size_{1};
            std::string font_family_, font_weight_, text_;
    };

    class ObjectContainer {
        public:
            virtual ~ObjectContainer() = default;
            virtual void AddPtr(std::unique_ptr<Object>&&) = 0;
            template <typename ObjectType>
            void Add(ObjectType obj) {AddPtr(std::make_unique<ObjectType>(std::move(obj)));}

        protected:
            std::vector<std::unique_ptr<Object>> storage_;
    };

    class Document final : public ObjectContainer {
        public:
            void AddPtr(std::unique_ptr<Object>&& object) override;
            void Render(std::ostream& out) const;
    };

    class Drawable {
        public:
            virtual ~Drawable() = default;
            virtual void Draw(svg::ObjectContainer&) const = 0;
    };

}  // namespace svg

namespace shapes {
    class Star : public svg::Drawable {
        public:
            Star(svg::Point center, double outer_radius, double inner_radius, int rays_count) : center_(center), outer_radius_(outer_radius), inner_radius_(inner_radius), rays_count_(rays_count) {}
            void Draw(svg::ObjectContainer& container) const override;

        private:
            svg::Point center_;
            double outer_radius_{0.}, inner_radius_{0.};
            int rays_count_{0};
            svg::Color fill_color_{"red"}, stroke_color_{"black"};
    };

    class Snowman : public svg::Drawable {
        public:
            Snowman(svg::Point head_center, double head_radius) : head_center_(head_center), head_radius_(head_radius) {}
            void Draw(svg::ObjectContainer& container) const override;

        private:
            svg::Point head_center_;
            double head_radius_;
            svg::Color fill_color_{"rgb(240,240,240)"}, stroke_color_{"black"};
    };

    class Triangle : public svg::Drawable {
        public:
            Triangle(svg::Point first, svg::Point second, svg::Point third) : first_(first), second_(second), third_(third) {}
            void Draw(svg::ObjectContainer& container) const override;

        private:
            svg::Point first_, second_, third_;
    };

}  // namespace shapes

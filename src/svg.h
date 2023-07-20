#pragma once
#define _USE_MATH_DEFINES

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <cmath>
#include <variant>

namespace svg {

class Rgb {
public:
    Rgb() = default;
    Rgb(uint8_t red, uint8_t green, uint8_t blue);
    
    uint8_t red_ = 0;
    uint8_t green_ = 0;
    uint8_t blue_ = 0;
};
inline void print_color(std::ostream& out, Rgb& rgb);
 
class Rgba {
public:
    Rgba() = default;
    Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity);
    
    uint8_t red_ = 0;
    uint8_t green_ = 0;
    uint8_t blue_ = 0;
    double opacity_ = 1.0;
};  
inline void print_color(std::ostream& out, Rgba& rgba);

using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
inline const Color NoneColor{"none"};  
    
inline void print_color(std::ostream& out, std::monostate);
inline void print_color(std::ostream& out, std::string& color);
std::ostream& operator<<(std::ostream& out, const Color& color); 
    
enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};
    
inline std::ostream &operator<<(std::ostream &out, StrokeLineCap stroke_line_cap) {
    using namespace std::literals;
    
    if (stroke_line_cap == StrokeLineCap::BUTT) {
        out << "butt"sv;
    } else if (stroke_line_cap == StrokeLineCap::ROUND) {
        out << "round"sv;
    } else if (stroke_line_cap == StrokeLineCap::SQUARE) {
        out << "square"sv;
    }
    return out;
}
 
enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};
    
inline std::ostream &operator<<(std::ostream &out, StrokeLineJoin stroke_line_join) {
    using namespace std::literals;
    
    if (stroke_line_join == StrokeLineJoin::ARCS) {
        out << "arcs"sv;
    } else if (stroke_line_join == StrokeLineJoin::BEVEL) {
        out << "bevel"sv;
    } else if (stroke_line_join == StrokeLineJoin::MITER) {
        out << "miter"sv;
    } else if (stroke_line_join == StrokeLineJoin::MITER_CLIP) {
        out << "miter-clip"sv;
    } else if (stroke_line_join == StrokeLineJoin::ROUND) {
        out << "round"sv;
    }
    return out;
}
    
template<typename Owner>
class PathProps {
public:
    Owner& set_fill_color(const Color& color) {
        fill_color_ = std::move(color);
        return as_owner();
    }
 
    Owner& set_stroke_color(const Color& color) {
        stroke_color_ = std::move(color);
        return as_owner();
    }
 
    Owner& set_stroke_width(double width) {
        stroke_width_ = width;
        return as_owner();
    }
 
    Owner& set_stroke_linecap(StrokeLineCap line_cap) {
        stroke_line_cap_ = line_cap;
        return as_owner();
    }
 
    Owner& set_stroke_linejoin(StrokeLineJoin line_join) {
        stroke_line_join_ = line_join;
        return as_owner();
    } 
 
protected:
    ~PathProps() = default;
 
    void render_attrs(std::ostream &out) const {
        using namespace std::literals;
 
        if (fill_color_ != std::nullopt) {
            out << "fill=\""sv << *fill_color_ << "\" "sv;
        }
        if (stroke_color_ != std::nullopt) {
            out << "stroke=\""sv << *stroke_color_ << "\" "sv;
        }
        if (stroke_width_ != std::nullopt) {
            out << "stroke-width=\""sv << *stroke_width_ << "\" "sv;
        }
        if (stroke_line_cap_ != std::nullopt) {
            out << "stroke-linecap=\""sv << *stroke_line_cap_ << "\" "sv;
        }
        if (stroke_line_join_ != std::nullopt) {
            out << "stroke-linejoin=\""sv << *stroke_line_join_ << "\" "sv;
        }
    }
    
private: 
    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<double> stroke_width_;
    std::optional<StrokeLineCap> stroke_line_cap_;
    std::optional<StrokeLineJoin> stroke_line_join_;
    
    Owner& as_owner() {
        return static_cast<Owner&>(*this);
    }
};
    
struct Point {
    Point() = default;
    Point(double x, double y) : x(x)
                              , y(y) {}
    double x = 0;
    double y = 0;
};

struct RenderContext {
    RenderContext(std::ostream& out);
    RenderContext(std::ostream& out, int indent_step, int indent = 0) : out_(out)
                                                                      , indent_step_(indent_step)
                                                                      , indent_(indent) {}
    RenderContext indented() const;
    void render_indent() const;

    std::ostream& out_;
    int indent_step_ = 0;
    int indent_ = 0;
};

class Object {
public:
    void render(const RenderContext& context) const;
    virtual ~Object() = default;

private:
    virtual void render_object(const RenderContext& context) const = 0;
};

class Circle final : public Object, public PathProps<Circle> {
public:
    Circle& set_center(Point center);
    Circle& set_radius(double radius);

private:
    Point center_;
    double radius_ = 1.0;
    
    void render_object(const RenderContext& context) const override;
};

class Polyline final : public Object, public PathProps<Polyline> {
public:
    Polyline& add_point(Point point);
    
private:
    std::vector<Point> points_;
    void render_object(const RenderContext& context) const override;
};

class Text final : public Object, public PathProps<Text> {
public:
    Text& set_position(Point pos);
    Text& set_offset(Point offset);
    Text& set_font_size(uint32_t size);
    Text& set_font_family(std::string font_family);
    Text& set_font_weight(std::string font_weight);
    Text& set_data(std::string data);

private:
    Point position_;
    Point offset_;
    std::string font_family_;
    std::string font_weight_;
    uint32_t font_size_ = 1;
    std::string data_;
     
    static std::string delete_spaces(const std::string& str);
    static std::string uniq_symbols(const std::string& str);
    
    void render_object(const RenderContext& context) const override;
};

class ObjectContainer {
public:
    virtual ~ObjectContainer() = default;
 
    template<typename Obj>
    void add(Obj obj);
 
    virtual void add_ptr(std::unique_ptr<Object> &&) = 0;
 
protected:
    std::vector<std::unique_ptr<Object>> objects_;
};
    
template<typename Obj>
void ObjectContainer::add(Obj obj) {
    objects_.emplace_back(std::make_unique<Obj>(std::move(obj)));
}
    
class Drawable {
public:
    virtual void draw(ObjectContainer& container) const = 0;
    virtual ~Drawable() = default;
};
    
class Document : public ObjectContainer {
public: 
    
    void add_ptr(std::unique_ptr<Object>&& obj) override {
        objects_.emplace_back(std::move(obj));
    }
    
    void render(std::ostream& out) const;
    
};
    
}//end namespace svg
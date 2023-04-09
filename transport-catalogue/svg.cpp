#define _USE_MATH_DEFINES

#include "svg.h"

#include <cmath>
#include <sstream>
#include <string_view>

namespace svg {
    using namespace std::literals;

    void ColorPrinter::operator()(std::monostate) const {
        os << NoneColor;
    }

    void ColorPrinter::operator()(const std::string& color) const {
        os << color;
    }

    void ColorPrinter::operator()(Rgb color) const {
        os << "rgb("s << std::to_string(color.red) << ","s << std::to_string(color.green) << "," << std::to_string(color.blue) << ")"s;
    }

    void ColorPrinter::operator()(Rgba color) const {
        os << "rgba("s << std::to_string(color.red) << ","s << std::to_string(color.green) << ","s << std::to_string(color.blue) << ","s;
        os << color.opacity << ")"s;
    }

    std::ostream& operator<<(std::ostream& os, const Color& color) {
        std::visit(ColorPrinter{os}, color);
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const StrokeLineCap& value) {
        switch (value) {
            case StrokeLineCap::BUTT:
                return os << "butt"s;
            case StrokeLineCap::ROUND:
                return os << "round"s;
            case StrokeLineCap::SQUARE:
                return os << "square"s;
        }
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const StrokeLineJoin& value) {
        switch (value) {
            case StrokeLineJoin::ARCS:
                return os << "arcs"s;
            case StrokeLineJoin::BEVEL:
                return os << "bevel"s;
            case StrokeLineJoin::MITER:
                return os << "miter"s;
            case StrokeLineJoin::MITER_CLIP:
                return os << "miter-clip"s;
            case StrokeLineJoin::ROUND:
                return os << "round"s;
        }
        return os;
    }

    RenderContext RenderContext::Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    void RenderContext::RenderIndent() const {
        for (int i = 0; i < indent; ++i){
            out.put(' ');
        }
    }

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();
        RenderObject(context);
        context.out << std::endl;
    }

    void Object::Render(std::ostream& os) const {
        RenderContext context(os, 2, 2);
        Render(context);
    }

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    Polyline& Polyline::AddPoint(Point point) {
        vertexes_.emplace_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\"";
        int id{0};
        for (const auto& vertex : vertexes_) {
            if (id++ != 0){
                out << " ";
            }
            out << vertex.x << ","sv << vertex.y;
        }
        out << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    Text& Text::SetPosition(Point position) {
        position_ = position;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = std::move(font_family);
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = std::move(font_weight);
        return *this;
    }

    Text& Text::SetData(std::string data) {
        text_ = std::move(data);
        return *this;
    }

    std::string Text::PreprocessTest(const std::string& input_text) const {
        std::string result = input_text;

        for (const auto& pair : kEscapeCharacters) {
            size_t position{0};
            while (true) {
                position = result.find(pair.character, position);
                if (position == std::string::npos)
                    break;
                result.replace(position, 1, pair.replacement);
                position = position + pair.replacement.size();
            }
        }

        return result;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text"sv;

        RenderAttrs(out);
        out << " ";
        out << "x=\""sv << position_.x << "\" y=\"" << position_.y << "\" "sv;
        out << "dx=\""sv << offset_.x << "\" dy=\"" << offset_.y << "\""sv;

        out << " font-size=\"" << font_size_ << "\"";
        if (!font_family_.empty())
            out << " font-family=\"" << font_family_ << "\"";
        if (!font_weight_.empty())
            out << " font-weight=\"" << font_weight_ << "\"";

        out << ">" << PreprocessTest(text_) << "</text>";
    }

    void Document::AddPtr(std::unique_ptr<Object>&& object) {
        storage_.emplace_back(std::move(object));
    }

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        for (const auto& object : storage_){
            object->Render(out);
        }
        out << "</svg>"sv;
    }

}  // namespace svg
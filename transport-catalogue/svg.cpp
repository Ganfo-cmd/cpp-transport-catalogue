#include "svg.h"

#include <unordered_map>

namespace svg
{
    using namespace std::literals;

    std::ostream &operator<<(std::ostream &out, const Color &color)
    {
        std::visit(RenderColor{out}, color);
        return out;
    }

    std::ostream &operator<<(std::ostream &out, StrokeLineCap line_cap)
    {
        using namespace std::literals;

        switch (line_cap)
        {
        case StrokeLineCap::BUTT:
            out << "butt"sv;
            break;

        case StrokeLineCap::ROUND:
            out << "round"sv;
            break;

        case StrokeLineCap::SQUARE:
            out << "square"sv;
            break;
        }
        return out;
    }

    std::ostream &operator<<(std::ostream &out, StrokeLineJoin line_join)
    {
        using namespace std::literals;

        switch (line_join)
        {
        case StrokeLineJoin::ARCS:
            out << "arcs"sv;
            break;

        case StrokeLineJoin::BEVEL:
            out << "bevel"sv;
            break;

        case StrokeLineJoin::MITER:
            out << "miter"sv;
            break;

        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip"sv;
            break;

        case StrokeLineJoin::ROUND:
            out << "round"sv;
            break;
        }
        return out;
    }

    void Object::Render(const RenderContext &context) const
    {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    //  ---------- Document ------------------

    void Document::AddPtr(std::unique_ptr<Object> &&obj)
    {
        objects_.push_back(std::move(obj));
    }

    void Document::Render(std::ostream &out) const
    {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        RenderContext rc(out, 2, 2);
        for (const auto &obj : objects_)
        {
            obj->Render(rc);
        }
        out << "</svg>"sv;
    }

    // ---------- Circle ------------------

    Circle &Circle::SetCenter(Point center)
    {
        center_ = center;
        return *this;
    }

    Circle &Circle::SetRadius(double radius)
    {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext &context) const
    {
        auto &out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        RenderOptionalAttrs(context.out);
        out << "/>"sv;
    }

    // ---------- Polyline ------------------

    Polyline &Polyline::AddPoint(Point point)
    {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext &context) const
    {
        auto &out = context.out;
        out << "<polyline points=\""sv;

        const auto end = points_.end();
        for (auto it = points_.begin(); it != end; ++it)
        {
            out << it->x << ","sv << it->y;
            if (next(it) != end)
            {
                out << " "sv;
            }
        }
        out << "\" ";
        RenderOptionalAttrs(context.out);
        out << "/>"sv;
    }

    // ---------- Text ------------------

    Text &Text::SetPosition(Point pos)
    {
        pos_ = pos;
        return *this;
    }

    Text &Text::SetOffset(Point offset)
    {
        offset_ = offset;
        return *this;
    }

    Text &Text::SetFontSize(uint32_t size)
    {
        size_ = size;
        return *this;
    }

    Text &Text::SetFontFamily(std::string font_family)
    {
        font_family_ = std::move(font_family);
        return *this;
    }

    Text &Text::SetFontWeight(std::string font_weight)
    {
        font_weight_ = std::move(font_weight);
        return *this;
    }

    Text &Text::SetData(std::string data)
    {
        data_ = std::move(data);
        return *this;
    }

    void RenderData(std::ostream &out, std::string_view data)
    {
        const std::unordered_map<char, std::string_view> speshial_chars = {{'"', "&quot;"sv}, {'<', "&lt;"}, {'>', "&gt;"}, {'\'', "&apos;"}, {'&', "&amp;"}};
        for (const auto &c : data)
        {
            const auto it = speshial_chars.find(c);
            if (it != speshial_chars.end())
            {
                out << it->second;
            }
            else
            {
                out << c;
            }
        }
    }

    void Text::RenderObject(const RenderContext &context) const
    {
        auto &out = context.out;
        out << "<text x=\""sv << pos_.x << "\" y=\"" << pos_.y << "\" dx=\"";
        out << offset_.x << "\" dy=\"" << offset_.y << "\" font-size=\"" << size_;
        out << "\"";
        if (!font_family_.empty())
        {
            out << " font-family=\"" << font_family_ << "\"";
        }

        if (!font_weight_.empty())
        {
            out << " font-weight=\"" << font_weight_ << "\"";
        }
        RenderOptionalAttrs(context.out);
        out << ">";

        RenderData(out, data_);
        out << "</text>";
    }

} // namespace svg
#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector> 
#include <algorithm> 
#include <optional>
#include <variant>
#include <sstream>

namespace svg {


    struct Rgb {
        Rgb() {}
        Rgb(uint8_t red_, uint8_t green_, uint8_t blue_) : red(red_), green(green_), blue(blue_) {}
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };

    struct Rgba {
        Rgba() {}
        Rgba(uint8_t red_, uint8_t green_, uint8_t blue_, double opacity_) : red(red_), green(green_), blue(blue_), opacity(opacity_) {}
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1;
    };


    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
    using namespace std::literals;

    struct ColorPrinter {
        std::ostream& out;
        void operator()(std::monostate) const {
            out << "none"s;
        }
        void operator()(std::string str) const {
            out << str;
        }
        void operator()(Rgb rgb) const {
            std::stringstream str;
            out << "rgb(" << std::to_string(rgb.red) << "," << std::to_string(rgb.green) << "," << std::to_string(rgb.blue) << ")";

        }
        void operator()(Rgba rgba) const {
            out << "rgba(" << std::to_string(rgba.red) << "," << std::to_string(rgba.green) << "," << std::to_string(rgba.blue) << "," << rgba.opacity << ")";
        }
    };



    // ������� � ������������ ����� ��������� �� �������������� inline,
    // �� ������� ���, ��� ��� ����� ����� �� ��� ������� ����������,
    // ������� ���������� ���� ���������.
    // � ��������� ������ ������ ������� ���������� ����� ������������ ���� ����� ���� ���������
    inline const Color NoneColor{"none"};

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap);

    std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join);

    std::ostream& operator<<(std::ostream& out, Color color);

    template <typename Owner>
    class PathProps {
    public:
        Owner& SetFillColor(Color color) {
            fill_color_ = std::move(color);
            return AsOwner();
        }
        Owner& SetStrokeColor(Color color) {
            stroke_color_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeWidth(double width) {
            width_ = width;
            return AsOwner();
        }

        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            line_cap_ = line_cap;
            return AsOwner();
        }

        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            line_join_ = line_join;
            return AsOwner();
        }




    protected:
        ~PathProps() = default;

        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;

            if (fill_color_) {
                out << " fill=\""sv << *fill_color_ << "\""sv;
            }
            if (stroke_color_) {
                out << " stroke=\""sv << *stroke_color_ << "\""sv;
            }
            if (width_) {
                out << " stroke-width=\""sv << *width_ << "\""sv;
            }
            if (line_cap_) {
                out << " stroke-linecap=\""sv << *line_cap_ << "\""sv;
            }
            if (line_join_) {
                out << " stroke-linejoin=\""sv << *line_join_ << "\""sv;
            }
        }

    private:
        Owner& AsOwner() {
            // static_cast ��������� ����������� *this � Owner&,
            // ���� ����� Owner � ��������� PathProps
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> width_;
        std::optional<StrokeLineCap> line_cap_;
        std::optional<StrokeLineJoin> line_join_;
    };






    class Object;

    class ObjectContainer {

    public:
        template <typename Obj>
        void Add(Obj obj) {
            AddPtr(std::make_unique<Obj>(std::move(obj)));
        }

        virtual ~ObjectContainer() = default;

    private:
        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
    };

    class Document final :public ObjectContainer {
    public:
        size_t Size() {
            return objects_.size();
        }

        // ������� � ostream svg-������������� ���������
        void Render(std::ostream& out) const;
        //  private:
        std::vector<std::unique_ptr<Object>> objects_;
        // ��������� � svg-�������� ������-��������� svg::Object
        void AddPtr(std::unique_ptr<Object>&& obj) override {
            objects_.emplace_back(std::move(obj));
        }
    };


    class Drawable {
    public:
        virtual void Draw(ObjectContainer& obj_cont) const = 0;
        virtual ~Drawable() = default;
    };

    struct Point {
        Point() = default;
        Point(double x, double y)
            : x(x)
            , y(y) {
        }
        double x = 0;
        double y = 0;
    };

    /*
     * ��������������� ���������, �������� �������� ��� ������ SVG-��������� � ���������.
     * ������ ������ �� ����� ������, ������� �������� � ��� ������� ��� ������ ��������
     */
    struct RenderContext {
        RenderContext(std::ostream& out)
            : out(out) {
        }

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
            : out(out)
            , indent_step(indent_step)
            , indent(indent) {
        }

        RenderContext Indented() const {
            return { out, indent_step, indent + indent_step };
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    /*
     * ����������� ������� ����� Object ������ ��� ���������������� ��������
     * ���������� ����� SVG-���������
     * ��������� ������� "��������� �����" ��� ������ ����������� ����
     */
    class Object {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    /*
     * ����� Circle ���������� ������� <circle> ��� ����������� �����
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
     */
    class Circle final : public Object, public PathProps<Circle> {

    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_;
        double radius_ = 1.0;
    };

    /*
     * ����� Polyline ���������� ������� <polyline> ��� ����������� ������� �����
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
     */
    class Polyline final : public Object, public PathProps<Polyline> {

    public:
        // ��������� ��������� ������� � ������� �����
        Polyline& AddPoint(Point point);

    private:
        std::vector<Point> points_;
        void RenderObject(const RenderContext& context) const override;
        void PrintPoints(std::ostream& out) const;
    };

    /*
     * ����� Text ���������� ������� <text> ��� ����������� ������
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
     */
    class Text final : public Object, public PathProps<Text> {
    public:
        // ����� ���������� ������� ����� (�������� x � y)
        Text& SetPosition(Point pos);

        // ����� �������� ������������ ������� ����� (�������� dx, dy)
        Text& SetOffset(Point offset);

        // ����� ������� ������ (������� font-size)
        Text& SetFontSize(uint32_t size);

        // ����� �������� ������ (������� font-family)
        Text& SetFontFamily(std::string font_family);

        // ����� ������� ������ (������� font-weight)
        Text& SetFontWeight(std::string font_weight);

        // ����� ��������� ���������� ������� (������������ ������ ���� text)
        Text& SetData(std::string data);

    private:
        Point pos_ = { 0,0 };
        Point offset_ = { 0,0 };
        uint32_t font_size_ = 1;
        std::string font_family_;
        std::string font_weight_;
        std::string data_;

        std::string Substitute(const std::string& sv) const {
            std::string res;
            std::for_each(sv.begin(), sv.end(), [&res](char c) {
                if (c == '&')
                    res += "&amp;";
                else if (c == '"')
                    res += "&quot;";
                else if (c == '\'' || c == '`')
                    res += "&apos;";
                else if (c == '<')
                    res += "&lt;";
                else if (c == '>')
                    res += "&gt;";
                else
                    res += c;
                });
            return res;
        }
        void RenderObject(const RenderContext& context) const override;
    };
}  // namespace svg

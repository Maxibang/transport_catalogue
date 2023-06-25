#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>

namespace svg {

using namespace std::literals;

enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE
};

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND
};


std::ostream& operator<< (std::ostream& out, const StrokeLineCap& line_cap);
std::ostream& operator<< (std::ostream& out, const StrokeLineJoin& line_join);

using Color = std::string;
inline const Color NoneColor{ "none" };


// Uses to set additional properties SVG shapes: fill, stroke, stroke-width, stroke-linecap, stroke-linejoin
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
    stroke_width_ = width;
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
        if (stroke_width_) {
            out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
        }
        if (line_cap_) {
            out << " stroke-linecap=\""sv;
            out << *line_cap_;
            out << "\""sv;
        }
        if (line_join_) {
            out << " stroke-linejoin=\""sv;
            out << *line_join_;
            out << "\""sv;
        }
    }

private:
    Owner& AsOwner() {
    // static_cast безопасно преобразует *this к Owner&,
    // если класс Owner — наследник PathProps
    return static_cast<Owner&>(*this);
    }

    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<double> stroke_width_;
    std::optional<StrokeLineCap> line_cap_;
    std::optional<StrokeLineJoin> line_join_;
        
}; // End of class PathProps


struct Point {
    Point() = default;
    Point(double x, double y) : x(x), y(y) {}
    double x = 0;
    double y = 0;
    
}; // End of struct Point


/* Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
* Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента*/
struct RenderContext {
    RenderContext(std::ostream& out)
    : out(out) {
    }

    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {}

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

}; // End of struct RenderContext


/*
* Абстрактный базовый класс Object служит для унифицированного хранения
* конкретных тегов SVG-документа
* Реализует паттерн "Шаблонный метод" для вывода содержимого тега
*/
class Object {
public:
    Object() = default;
    virtual ~Object() = default;
    virtual void Render(const RenderContext& context) const;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;

}; // End of class Object


/*
* Класс Circle моделирует элемент <circle> для отображения круга
* https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
*/
class Circle final : public Object, public PathProps<Circle> {
public:
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);
    Circle() = default;
    ~Circle() = default;

private:
    void RenderObject(const RenderContext& context) const override;
    Point center_;
    double radius_ = 1.0;

}; // End of class Object


/*
* Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
* https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
*/
class Polyline final : public Object, public PathProps<Polyline> {
public:
    // Добавляет очередную вершину к ломаной линии
    // Realiazed in SVG.cpp
    Polyline& AddPoint(Point point);
    Polyline() = default;
    ~Polyline() = default;
private:
    // Realiazed in SVG.cpp
    void RenderObject(const RenderContext& context) const override;
    std::vector<Point> points_;

}; // End of class Circle


/*
* Класс Text моделирует элемент <text> для отображения текста
* https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
*/
class Text final : public Object, public PathProps<Text> {
public:
    Text() = default;
    ~Text() = default;
    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& SetPosition(Point pos);

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(Point offset);

    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t size);

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string data);

private:
    void RenderObject(const RenderContext& context) const override;
    std::string data_ = ""s;
    std::string font_weight_ = ""s;
    std::string font_family_ = ""s;
    uint32_t font_size_ = 1;
    Point offset_ = { 0, 0 };
    Point position_ = { 0, 0 };

}; // End of class Text


// Collects collection of DrawAble objects
class ObjectContainer {
public:
    ObjectContainer() = default;
    virtual ~ObjectContainer() = default;
    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
    
    template <typename ObjectType>
    void Add(ObjectType object) {
        documents_.push_back(std::make_unique<ObjectType>(std::move(object)));
    }

protected:
    std::vector<std::unique_ptr<Object>> documents_;

}; // End of class ObjectContainer


// Interface for objects which can be drawed 
class Drawable {
public:
    virtual void Draw(svg::ObjectContainer& container) const = 0;
    virtual ~Drawable() = default;

};   // End of class Drawable 


class Document : public ObjectContainer {
public:
    
    /*Метод Add добавляет в svg-документ любой объект-наследник svg::Object.
    Пример использования:
    Document doc;
    doc.Add(Circle().SetCenter({20, 30}).SetRadius(15)); */
    // void Add(???);
    Document() = default;

    // Добавляет в svg-документ объект-наследник svg::Object
    void AddPtr(std::unique_ptr<Object>&& obj) override;

    // Выводит в ostream svg-представление документа
    void Render(std::ostream& out) const;

}; // End of class Document


template <typename DrawableIterator>
    void DrawPicture(DrawableIterator begin, DrawableIterator end, svg::ObjectContainer& target) {
    for (auto it = begin; it != end; ++it) {
        (*it)->Draw(target);
    }
}


template <typename Container>
void DrawPicture(const Container& container, svg::ObjectContainer& target) {
    using namespace std;
    DrawPicture(begin(container), end(container), target);
}

}  // namespace svg
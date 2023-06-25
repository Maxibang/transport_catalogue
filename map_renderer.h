#pragma once

#include "geo.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "json.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <sstream>


using namespace transport;
using namespace transport::detail;
using namespace transport::detail;
using namespace transport::catalogue;
using namespace std::string_literals;
using namespace json;


inline const double EPSILON = 1e-6;
bool IsZero(double value);

class SphereProjector {
public:
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width, double max_height, double padding);

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(transport::detail::Coordinates coords) const;

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};


// Constructor of SphereProjector class
// points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
template <typename PointInputIt>
SphereProjector::SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width, double max_height, double padding) : padding_(padding) {
    
    // Если точки поверхности сферы не заданы, вычислять нечего
    if (points_begin == points_end) {
        return;
    }

    // Находим точки с минимальной и максимальной долготой
    const auto [left_it, right_it] = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
    min_lon_ = left_it->lng;
    const double max_lon = right_it->lng;

    // Находим точки с минимальной и максимальной широтой
    const auto [bottom_it, top_it] = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
    const double min_lat = bottom_it->lat;
    max_lat_ = top_it->lat;

// Вычисляем коэффициент масштабирования вдоль координаты x
    std::optional<double> width_zoom;
    if (!IsZero(max_lon - min_lon_)) {
        width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
    }

    // Вычисляем коэффициент масштабирования вдоль координаты y
    std::optional<double> height_zoom;
    if (!IsZero(max_lat_ - min_lat)) {
        height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
    }

    if (width_zoom && height_zoom) {
        // Коэффициенты масштабирования по ширине и высоте ненулевые,
        // берём минимальный из них
        zoom_coeff_ = std::min(*width_zoom, *height_zoom);
    }
    else if (width_zoom) {
        // Коэффициент масштабирования по ширине ненулевой, используем его
        zoom_coeff_ = *width_zoom;
    }
    else if (height_zoom) {
        // Коэффициент масштабирования по высоте ненулевой, используем его
        zoom_coeff_ = *height_zoom;
    }
}


/*
    Структура словаря render_settings:

    width и height — ширина и высота изображения в пикселях. Вещественное число в диапазоне от 0 до 100000.
    padding — отступ краёв карты от границ SVG-документа. Вещественное число не меньше 0 и меньше min(width, height)/2.
    line_width — толщина линий, которыми рисуются автобусные маршруты. Вещественное число в диапазоне от 0 до 100000.
    stop_radius — радиус окружностей, которыми обозначаются остановки. Вещественное число в диапазоне от 0 до 100000.
    bus_label_font_size — размер текста, которым написаны названия автобусных маршрутов. Целое число в диапазоне от 0 до 100000.
    bus_label_offset — смещение надписи с названием маршрута относительно координат конечной остановки на карте. 
    Массив из двух элементов типа double. Задаёт значения свойств dx и dy SVG-элемента <text>. Элементы массива — числа в диапазоне от –100000 до 100000.
    stop_label_font_size — размер текста, которым отображаются названия остановок. Целое число в диапазоне от 0 до 100000.
    stop_label_offset — смещение названия остановки относительно её координат на карте. 
    Массив из двух элементов типа double. Задаёт значения свойств dx и dy SVG-элемента <text>. Числа в диапазоне от –100000 до 100000.
    underlayer_color — цвет подложки под названиями остановок и маршрутов. Формат хранения цвета будет ниже.
    underlayer_width — толщина подложки под названиями остановок и маршрутов. 
    Задаёт значение атрибута stroke-width элемента <text>. Вещественное число в диапазоне от 0 до 100000.
    color_palette — цветовая палитра. Непустой массив.

Цвет можно указать в одном из следующих форматов:
     - в виде строки, например, "red" или "black";
     - в массиве из трёх целых чисел диапазона [0, 255]. 
       Они определяют r, g и b компоненты цвета в формате svg::Rgb. Цвет [255, 16, 12] нужно вывести в SVG как rgb(255,16,12);
     - в массиве из четырёх элементов: три целых числа в диапазоне от [0, 255] и одно вещественное число в диапазоне от [0.0, 1.0]. 
       Они задают составляющие red, green, blue и opacity цвета формата svg::Rgba. 
       Цвет, заданный как [255, 200, 23, 0.85], должен быть выведен в SVG как rgba(255,200,23,0.85).
Гарантируется, что каждый цвет задан в одном из этих трёх форматов.
*/


// struct RenderSettings stores render settings and allow get color as string by using member-function
struct RenderSettings {

    double width = 0;
    double height = 0;

    double padding = 0;

    double line_width = 0;
    double stop_radius = 0;

    int bus_label_font_size = 0;
    svg::Point bus_label_offset = {0, 0};

    double stop_label_font_size = 0;
    svg::Point stop_label_offset = { 0, 0 };

    std::string underlayer_color;
    double underlayer_width = 0;

    std::vector<std::string> color_palette;
    std::string ColorPalette(const int index) const;
    std::string UnderlayerColor() const;
};


// Save render settings from Document (loaded from JSON) into RenderSettings struct
RenderSettings SaveRenderSettings(const Document& document);

// Get Color in string format from json Node
std::string GetColorFromNode(const Node& color);


/// *** Classes for drawing bus route MAP *** ///

// Class for drawing route lines
class Route : public svg::Drawable {
public:
    Route(svg::Polyline& route_line) : route_line_(std::move(route_line)) {}

    void Draw(svg::ObjectContainer& container) const override;

private:
    svg::Polyline route_line_;
};


// Class for drawing text
class TextDraw : public svg::Drawable {
public:
    TextDraw(svg::Text& text) : text(text) {}

    void Draw(svg::ObjectContainer& container) const override;

private:
    svg::Text text;
};


// Class for drawing stops points
class PointDraw : public svg::Drawable {
public:
    PointDraw(svg::Circle& circle) : circle_(std::move(circle)) {}

    void Draw(svg::ObjectContainer& container) const override;

private:
    svg::Circle circle_;
};

/// *** END OF Classes for drawing bus route MAP *** ///


// Function that forms all Drawable figures for route picture
std::vector<std::unique_ptr<svg::Drawable>> DrawBusessRoutes(const RenderSettings& render_settings, const TransportCatalogue& catalogue);
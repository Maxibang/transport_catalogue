#include "map_renderer.h"

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
using namespace std;




static size_t RGB_NUMBER = 3;
static size_t RGBA_NUMBER = 4;

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}


// Проецирует широту и долготу в координаты внутри SVG-изображения
svg::Point SphereProjector::operator()(transport::detail::Coordinates coords) const {
    return {
        (coords.lng - min_lon_) * zoom_coeff_ + padding_,
        (max_lat_ - coords.lat) * zoom_coeff_ + padding_
    };
}


/// *** Struct RenderSettings *** ///

// Exception throw if GetColor can't finish properly
class ColorReadException : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};


std::string RenderSettings::UnderlayerColor() const {
    return underlayer_color;
}

std::string RenderSettings::ColorPalette(const int index) const {
    return color_palette.at(index % color_palette.size());
}

std::string GetColorFromNode(const Node& color) {
    if (color.IsString()) {
        return color.AsString();
    }
    if (color.AsArray().size() == RGB_NUMBER) {
        std::ostringstream out;
        out << "rgb("s << color.AsArray().at(0).AsInt() <<
            ","s << color.AsArray().at(1).AsInt() << ","s
            << color.AsArray().at(2).AsInt() << ")"s;
        return out.str();
    }

    if (color.AsArray().size() == RGBA_NUMBER) {
        std::ostringstream out;
        out << "rgba("s << color.AsArray().at(0).AsInt() << 
            ","s << color.AsArray().at(1).AsInt() << ","s <<
            color.AsArray().at(2).AsInt() << ","s <<
            color.AsArray().at(3).AsDouble() << ")"s;
        return out.str();
    }
    throw ColorReadException("Error: Array doesn't contain a proper color information!!!"s);
}


/// *** END OF Struct RenderSettings *** ///


// Save render settings from Document (loaded from JSON) into RenderSettings struct

RenderSettings SaveRenderSettings(const Document& document) {
    RenderSettings render_settings;
    
    if (document.GetRoot().AsMap().count("render_settings"s) == 0) {
        throw ColorReadException("No render_settings in document!!!");
    }
    
    const auto& settings_map = document.GetRoot().AsMap().at("render_settings"s).AsMap();
    
    // Fill render settings struct
    render_settings.width = settings_map.at("width"s).AsDouble();
    render_settings.height = settings_map.at("height"s).AsDouble();

    render_settings.padding = settings_map.at("padding"s).AsDouble();

    render_settings.line_width = settings_map.at("line_width"s).AsDouble();
    render_settings.stop_radius = settings_map.at("stop_radius"s).AsDouble();

    render_settings.bus_label_font_size = settings_map.at("bus_label_font_size"s).AsInt();
    render_settings.bus_label_offset = {settings_map.at("bus_label_offset"s).AsArray().at(0).AsDouble(), settings_map.at("bus_label_offset"s).AsArray().at(1).AsDouble()};

    render_settings.stop_label_offset = {settings_map.at("stop_label_offset"s).AsArray().at(0).AsDouble(), settings_map.at("stop_label_offset"s).AsArray().at(1).AsDouble()};
    render_settings.stop_label_font_size = settings_map.at("stop_label_font_size"s).AsInt();
        
    render_settings.underlayer_color = GetColorFromNode(settings_map.at("underlayer_color"s));
    render_settings.underlayer_width = settings_map.at("underlayer_width"s).AsDouble();
    
    // Fill vector<string> color_palette
    const auto& array = settings_map.at("color_palette"s).AsArray();
    
    const auto size = array.size();
    render_settings.color_palette.reserve(size);
    
    for (size_t i = 0; i < size; ++i) {
        render_settings.color_palette.push_back(std::move(GetColorFromNode(array.at(i))));
    }
    
    return render_settings;
}

/// *** Classes for drawing bus route MAP *** ///

// Class for drawing route lines
void Route::Draw(svg::ObjectContainer& container) const {
    container.Add(route_line_);
}


// Class for drawing text
void TextDraw::Draw(svg::ObjectContainer& container) const {
    container.Add(text);
}


// Class for drawing stops points
void PointDraw::Draw(svg::ObjectContainer& container) const {
    container.Add(circle_);
}

/// *** END OF Classes for drawing bus route MAP *** ///


// Return underlayer for bus name - USING IN DrawBusessRoutes FUNCTION
svg::Text BusNameUnderlayer(const std::string_view bus_name, 
    const TransportCatalogue& catalogue, const SphereProjector& projector, 
    const std::vector<std::string_view>& stops, const RenderSettings& render_settings) {

    svg::Text bus_name_underlayer;
    
    bus_name_underlayer.SetPosition(projector(catalogue.FindStop(stops.at(0)).coordinates));
    bus_name_underlayer.SetOffset(render_settings.bus_label_offset);
    bus_name_underlayer.SetFontSize(render_settings.bus_label_font_size);
    bus_name_underlayer.SetFontFamily("Verdana");
    bus_name_underlayer.SetFontWeight("bold");
    bus_name_underlayer.SetData(std::string(bus_name));
    bus_name_underlayer.SetFillColor(render_settings.UnderlayerColor());
    bus_name_underlayer.SetStrokeColor(render_settings.UnderlayerColor());
    bus_name_underlayer.SetStrokeWidth(render_settings.underlayer_width);
    bus_name_underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    bus_name_underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    
    return bus_name_underlayer;
}


// Return bus name for drawing - USING IN DrawBusessRoutes FUNCTION
svg::Text BusName(const std::string_view bus_name,
    const TransportCatalogue& catalogue, const SphereProjector& projector,
    const std::vector<std::string_view>& stops, const RenderSettings& render_settings, const int index) {
    
    svg::Text name_bus;
    
    name_bus.SetPosition(projector(catalogue.FindStop(stops.at(0)).coordinates));
    name_bus.SetOffset(render_settings.bus_label_offset);
    name_bus.SetFontSize(render_settings.bus_label_font_size);
    name_bus.SetFontFamily("Verdana");
    name_bus.SetFontWeight("bold");
    name_bus.SetData(std::string(bus_name));
    name_bus.SetFillColor(render_settings.ColorPalette(index));
    
    return name_bus;
}


// Return route line for drawing - USING IN DrawBusessRoutes FUNCTION
svg::Polyline DrawRouteLine(const std::vector<std::string_view>& stops, 
    const SphereProjector& projector, const TransportCatalogue& catalogue, 
    const RenderSettings& render_settings, const int index) {

    svg::Polyline route_line;
    
    // Add ponts for bus route draw
    for (const auto& stop : stops) {
        const auto Point = projector(catalogue.FindStop(stop).coordinates);
        route_line.AddPoint(Point);
    }

    // Route line attributes for draw (extra to points)  
    route_line.SetStrokeWidth(render_settings.line_width);
    route_line.SetFillColor(svg::NoneColor);
    route_line.SetStrokeColor(render_settings.ColorPalette(index));
    route_line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    route_line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    
    return route_line;
}


// Return a small circle picture for drawing point of stops - USING IN DrawBusessRoutes FUNCTION
svg::Circle DrawStopPoint(const Stop& stop, const SphereProjector& projector, const RenderSettings& render_settings) {
    
    svg::Circle circle;
    
    circle.SetCenter(projector(stop.coordinates));
    circle.SetRadius(render_settings.stop_radius);
    circle.SetFillColor("white");
    
    return circle;
}


// Return stop name underlayer for drawing - USING IN DrawBusessRoutes FUNCTION
svg::Text DrawStopNameUnderlayer(const Stop& stop, const SphereProjector& projector, const RenderSettings& render_settings) {

    svg::Text underlayer;

    underlayer.SetPosition(projector(stop.coordinates));
    underlayer.SetOffset(render_settings.stop_label_offset);
    underlayer.SetOffset(render_settings.stop_label_offset);
    underlayer.SetFontSize(render_settings.stop_label_font_size);
    underlayer.SetFontFamily("Verdana");
    underlayer.SetData(std::string(stop.stop_name));
    underlayer.SetFillColor(render_settings.UnderlayerColor());
    underlayer.SetStrokeColor(render_settings.UnderlayerColor());
    underlayer.SetStrokeWidth(render_settings.underlayer_width);
    underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    return underlayer;
}


// Return stop name for drawing - USING IN DrawBusessRoutes FUNCTION
svg::Text DrawStopName(const Stop& stop, const SphereProjector& projector,
    const RenderSettings& render_settings) {
    
    svg::Text stop_name;
    
    stop_name.SetPosition(projector(stop.coordinates));
    stop_name.SetOffset(render_settings.stop_label_offset);
    stop_name.SetFontSize(render_settings.stop_label_font_size);
    stop_name.SetFontFamily("Verdana");
    stop_name.SetData(std::string(stop.stop_name));
    stop_name.SetFillColor("black");

    return stop_name;

}


// Function that forms all Drawable figures for route picture
std::vector<unique_ptr<svg::Drawable>> DrawBusessRoutes(const RenderSettings& render_settings, const TransportCatalogue& catalogue) {
    
    // Fill set for store buses in sort-condition
    const auto& all_buses = catalogue.GetAllBuses();
    std::set<std::string_view> buses;
    for (const auto& bus : all_buses) {
        if (!bus.bus_route.empty()) {
            buses.insert(bus.bus_number);
        }
    }
    
    const std::deque<Stop>& stops = catalogue.GetAllStops();
    vector<Coordinates> alls_stops;
    alls_stops.reserve(stops.size());
    // List for drawing stops points and stops names
    std::set<std::string_view> stops_names;
    
    // Find all stops where at least one bus stops
    for (const auto& bus : buses) {
        for (const auto& stop : catalogue.FindBus(std::string(bus))) {
            alls_stops.push_back(catalogue.FindStop(stop).coordinates);
            stops_names.insert(stop);
        }
    }

    // Using for get rigth coordinates on plane
    SphereProjector projector(alls_stops.begin(), alls_stops.end(), render_settings.width, render_settings.height, render_settings.padding);
  
    // Routes to draw
    std::vector<unique_ptr<svg::Drawable>> routes;
    // Buses names to draw
    std::vector<unique_ptr<svg::Drawable>> buses_names;
    
    int index = 0;
    for (const auto& bus : buses) {
        const auto& stops = catalogue.FindBus(std::string(bus));
        
        // Route line for draw
        svg::Polyline route_line = DrawRouteLine(stops, projector, catalogue, render_settings, index);
        routes.push_back(std::make_unique<Route>(std::move(Route(route_line))));
        
        // Bus name underlayer for draw 
        svg::Text bus_name_underlayer = BusNameUnderlayer(bus, catalogue, projector, stops, render_settings);
        buses_names.push_back(std::make_unique<TextDraw>(TextDraw(bus_name_underlayer)));
        
        // Bus name for draw above underlayer
        svg::Text bus_name = BusName(bus, catalogue, projector, stops, render_settings, index);
        buses_names.push_back(std::make_unique<TextDraw>(TextDraw(bus_name)));
        
        // If bus_route isn't roundtrip - draw last stop of direct route
        if (!catalogue.FindBusPtr(bus)->is_roundtrip && stops.at(stops.size() / 2) != stops.at(0)) {
            svg::Text bus_name_underlayer = BusNameUnderlayer(bus, catalogue, projector, stops, render_settings);
            bus_name_underlayer.SetPosition(projector(catalogue.FindStop(stops.at(stops.size() / 2)).coordinates));
            buses_names.push_back(std::make_unique<TextDraw>(TextDraw(bus_name_underlayer)));

            svg::Text bus_name = BusName(bus, catalogue, projector, stops, render_settings, index);
            bus_name.SetPosition(projector(catalogue.FindStop(stops.at(stops.size() / 2)).coordinates));
            buses_names.push_back(std::make_unique<TextDraw>(TextDraw(bus_name)));
        }
        
        // Increment index to get next Color for bus names and route lines
        ++index;
    }
    
    // Write buses names into result vector 
    for (auto& name : buses_names) {
        routes.push_back(std::move(name));
    }
    
    // Stops names for drawing
    std::vector<unique_ptr<svg::Drawable>> names_of_stops;
    
    //  Stops points and stops names for draw
    for (const auto& stop_name : stops_names) {
        
        const auto& stop = catalogue.FindStop(stop_name);

        // Stops points
        svg::Circle circle = DrawStopPoint(stop, projector, render_settings);
        routes.push_back(std::make_unique<PointDraw>(std::move(PointDraw(circle))));
        
        // Stops name underlayer 
        svg::Text underlayer = DrawStopNameUnderlayer(stop, projector, render_settings);
        names_of_stops.push_back(std::make_unique<TextDraw>(TextDraw(underlayer)));
        
        // Stops name for draw above underlayer
        svg::Text name_stop = DrawStopName(stop, projector, render_settings);
        names_of_stops.push_back(std::make_unique<TextDraw>(TextDraw(name_stop)));
    }
                         
    // Write stops names into result vector
    for (auto& name : names_of_stops) {
        routes.push_back(std::move(name));
    }
    
    return routes;
}
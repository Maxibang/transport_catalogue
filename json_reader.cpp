#include "json_reader.h"
#include "json_builder.h"
#include <iostream>
#include "transport_router.h"

using namespace std;

// Adds info about Stop from using Dict = std::map<std::string, Node>; into catalogue
void AddStopIntoCatalogue(TransportCatalogue& catalogue, const Dict& stop_description_map, DstBetwStops& dist_to_stop) {
    double latitude = stop_description_map.at("latitude"s).AsDouble();
    double longitude = stop_description_map.at("longitude"s).AsDouble();
    std::string stop_name = stop_description_map.at("name"s).AsString();
    if (stop_description_map.count("road_distances"s) > 0) {
        dist_to_stop[stop_name] = stop_description_map.at("road_distances"s).AsMap();
    }

    catalogue.AddNewStop(std::move(stop_name), { latitude, longitude });
}


// Adds info about Bus from using Dict = std::map<std::string, Node>; into catalogue
void AddBusIntoCatalogue(TransportCatalogue& catalogue, const Dict& bus_description_map) {
    vector<string> stops;
    const vector<Node>& ref_vector = bus_description_map.at("stops"s).AsArray();
    stops.reserve(ref_vector.size() * 2);

    // Add straight route into stops from 1-st to 2-nd stop
    for (auto& stop : ref_vector) {
        stops.push_back(stop.AsString());
    }

    // Add stops into vec-stops in reverse range if route isn't round
    if (!bus_description_map.at("is_roundtrip"s).AsBool()) {
        for (int i = static_cast<int>(ref_vector.size()) - 2; i >= 0; --i) {
            stops.push_back(ref_vector.at(i).AsString());
        }
    }
    catalogue.AddNewBus(bus_description_map.at("name"s).AsString(), stops, bus_description_map.at("is_roundtrip"s).AsBool());
}


// Get info about Buses and Stops from struct Document and add it into TransportCatalogue
void FillTransportCatalogue(TransportCatalogue& catalogue, const Document& document) {
    const auto& json_map = document.GetRoot().AsMap();

    // Store Stop to Stop Distance Map which will process
    DstBetwStops dist_to_stop;

    // Process all Stops
    for (const auto& node : json_map.at("base_requests"s).AsArray()) {
        const Dict& base_content = node.AsMap();
        if (base_content.at("type"s) == "Stop"s) {
            AddStopIntoCatalogue(catalogue, base_content, dist_to_stop);
        }
    }

    // Process Stop To Stop Distances
    for (const auto& [stop_from, next_stop] : dist_to_stop) {
        for (const auto& [stop_to, dist] : next_stop) {
            catalogue.AddDstBetweenStops(stop_from, dist.AsInt(), stop_to);
        }
    }

    // Process all Buses
    for (const auto& node : json_map.at("base_requests"s).AsArray()) {
        const Dict& base_content = node.AsMap();
        if (base_content.at("type"s) == "Bus"s) {
            AddBusIntoCatalogue(catalogue, base_content);
        }
    }


    const auto& route_settings = json_map.at("routing_settings"s).AsMap();
    catalogue.SetBusVelocity(route_settings.at("bus_velocity"s).AsDouble());
    catalogue.SetBusWaitTime(route_settings.at("bus_wait_time"s).AsInt());
    
}

/* ///// **** END FILL DATA INTO CATALOGUE ****///// */


/* ///// **** READ REQUESTS FROM JSON AND FORM ANSWER ****///// */

/*
// Old version of ParseStopAnswer - before class Builder() existed
Node ParseStopAnswer(TransportCatalogue& catalogue, const Dict& request) {
    const Stop& stop = catalogue.FindStop(request.at("name"s).AsString());
    Dict dict_node;
    dict_node["request_id"s] = request.at("id"s);

    if (stop.IsEmtyStop()) {
        dict_node["error_message"s] = "not found"s;
        return dict_node;

    } else {
        const std::set<std::string_view>& all_stops =  catalogue.FindBusesAtStop(stop.stop_name);
        Array stops;
        stops.reserve(all_stops.size());

        for (auto& stop : all_stops) {
            stops.push_back({ std::string(stop) });
        }

        dict_node["buses"s] = std::move(stops);
        return dict_node;
    }
}
*/


// New version of ParseStopAnswer - class Builder() exists
Node ParseStopAnswer(TransportCatalogue& catalogue, const Dict& request) {
    const Stop& stop = catalogue.FindStop(request.at("name"s).AsString());
    json::Builder build_answer;
    build_answer.StartDict().Key("request_id"s).Value(request.at("id"s).AsInt());

    if (stop.IsEmtyStop()) {
        build_answer.Key("error_message"s).Value("not found"s).EndDict();
        return build_answer.Build();
    }
    else {
        const std::set<std::string_view>& all_stops = catalogue.FindBusesAtStop(stop.stop_name);

        build_answer.Key("buses"s).StartArray();
        for (auto& stop : all_stops) {
            build_answer.Value(std::string(stop));
        }

        return build_answer.EndArray().EndDict().Build();
    }
}


/*
// Old version of ParseBusAnswer - before class Builder() existed
Node ParseBusAnswer(const TransportCatalogue& catalogue, const Dict& request) {
    Dict dict_node;
    dict_node["request_id"s] = request.at("id"s);
    const vector<string_view>& bus = catalogue.FindBus(request.at("name"s).AsString());

    // if such bus-route doesn't exists
    if (bus.empty()) {
        dict_node["error_message"s] = "not found"s;
        return dict_node;
    }

    // Add info about bus-route
    dict_node["stop_count"s] = static_cast<int>(bus.size());
    std::set<std::string_view> unique_stop(bus.begin(), bus.end());
    dict_node["unique_stop_count"s] = static_cast<int>(unique_stop.size());
    const auto distances = catalogue.ComputeRouteDistance(bus);
    //cout << "distances.gps_dist"s << distances.gps_dist << endl;
    dict_node["curvature"s] = static_cast<double>(distances.real_dist / distances.gps_dist);
    dict_node["route_length"s] = distances.real_dist;
    return dict_node;
}
*/


// New version of ParseBusAnswer - class Builder() exists
Node ParseBusAnswer(const TransportCatalogue& catalogue, const Dict& request) {
    Builder build_answer;
    build_answer.StartDict().Key("request_id"s).Value(request.at("id"s).AsInt());
    const vector<string_view>& bus = catalogue.FindBus(request.at("name"s).AsString());

    // if such bus-route doesn't exists
    if (bus.empty()) {
        build_answer.Key("error_message"s).Value("not found"s).EndDict();
        return build_answer.Build();
    }

    // Add info about bus-route
    build_answer.Key("stop_count"s).Value(static_cast<int>(bus.size()));
    std::set<std::string_view> unique_stop(bus.begin(), bus.end());

    build_answer.Key("unique_stop_count"s).Value(static_cast<int>(unique_stop.size()));
    const auto distances = catalogue.ComputeRouteDistance(bus);

    build_answer.Key("curvature"s).Value(static_cast<double>(distances.real_dist / distances.gps_dist));
    build_answer.Key("route_length"s).Value(distances.real_dist).EndDict();

    return build_answer.Build();
}


/*
// Old version of ParseSvgBusRoute - before class Builder() existed
Node ParseSvgBusRoute(const TransportCatalogue& catalogue, const Dict& request, const Document& document) {
    Dict dict_node;
    dict_node["request_id"s] = request.at("id"s);
    std::ostringstream o_stream;

    // Here we draw buses routes in SVG format using map_renderer.h
    svg::Document doc;
    std::vector<unique_ptr<svg::Drawable>> pucture = std::move(DrawBusessRoutes(document, catalogue));
    DrawPicture(pucture, doc);
    doc.Render(o_stream);

    dict_node["map"s] = o_stream.str();
    return dict_node;
}
*/


// New version of ParseSvgBusRoute - class Builder() exists
Node ParseSvgBusRoute(const TransportCatalogue& catalogue, const Dict& request, const RenderSettings& render_settings) {
    Builder build_answer;
    build_answer.StartDict();
    build_answer.Key("request_id"s).Value(request.at("id"s).AsInt());

    // Here we draw buses routes in SVG format using map_renderer.h
    svg::Document doc;
    std::vector<unique_ptr<svg::Drawable>> pucture = std::move(DrawBusessRoutes(render_settings, catalogue));
    DrawPicture(pucture, doc);
    std::ostringstream o_stream;
    doc.Render(o_stream);

    build_answer.Key("map"s).Value(o_stream.str()).EndDict();

    return build_answer.Build();
}


// Parsing route answer via creating minimal route by using SingleBusRoute class (struct)
Node ParseRouteAnswer(const TransportCatalogue& catalogue, const Dict& request) {
    Builder build_answer;
    build_answer.StartDict();
    build_answer.Key("request_id"s).Value(request.at("id"s).AsInt());

    static SingleBusRoute tracker(catalogue);
    const auto route = tracker.BuildRoute(request.at("from"s).AsString(), request.at("to"s).AsString());

    if (route.has_value()) {
        build_answer.Key("total_time"s).Value((*route).weight);
        build_answer.Key("items"s).StartArray();

        const auto wait_time = catalogue.BusWaitTime();
        for (size_t edge : (*route).edges) {

            build_answer.StartDict().Key("stop_name"s).Value(tracker.GetStopNameByEdgeId(edge));
            build_answer.Key("time"s).Value(wait_time);
            build_answer.Key("type"s).Value("Wait"s).EndDict();

            build_answer.StartDict().Key("bus"s).Value(tracker.GetBusNameByEdgeId(edge));
            build_answer.Key("span_count"s).Value(tracker.GetStopNumbByEdgeId(edge));
            build_answer.Key("time"s).Value(tracker.GetEdgeWeightByEdgeId(edge) - wait_time);
            build_answer.Key("type"s).Value("Bus"s).EndDict();
        }
        build_answer.EndArray();
    }
    else {
        build_answer.Key("error_message"s).Value("not found"s);
    }

    build_answer.EndDict();

    return build_answer.Build();
}


// Get and build all answers from stat_requests Node from readed Json file
Node GetReaquestAnwer(TransportCatalogue& catalogue, const Document& document, const RenderSettings& render_settings) {
    const auto& json_map = document.GetRoot().AsMap();
    Array result_node;
    result_node.reserve(json_map.at("stat_requests"s).AsArray().size());

    for (const auto& node : json_map.at("stat_requests"s).AsArray()) {
        const Dict& base_content = node.AsMap();
        // Parse answer for stop-request
        if (base_content.at("type"s).AsString() == "Stop"s) {
            result_node.push_back(std::move(ParseStopAnswer(catalogue, base_content)));
        }
        // Parse answer for bus-request
        else if (base_content.at("type"s).AsString() == "Bus"s) {
            result_node.push_back(std::move(ParseBusAnswer(catalogue, base_content)));
        }
        else if (base_content.at("type"s).AsString() == "Map"s) {
            result_node.push_back(std::move(ParseSvgBusRoute(catalogue, base_content, render_settings)));
            // Here we process "Route" request. In Future we can unify request parametres and take it into map<request_type, function> 
        }
        else if (base_content.at("type"s).AsString() == "Route"s) {
            result_node.push_back(std::move(ParseRouteAnswer(catalogue, base_content)));
        }
    }

    return result_node;
}

/* ///// **** END FORM ANSWER ****///// */
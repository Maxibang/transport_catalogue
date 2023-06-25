#include "transport_router.h"
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace std;

using Bus = transport::detail::Bus;
using VertexId = size_t;
using RouteInfo = graph::Router<double>::RouteInfo;

// Fill graph with STRAIGHT bus trip info avoiding creating excessive edges
void SingleBusRoute::ProcessStraightBusRoute(const Bus& bus, const vector<string_view>& stops, size_t stops_size) {
    const auto bus_velocity = catalogue.BusVelocity();
    
    // Proccess all stops till middle
    for (int i = 0; i < stops_size / 2; ++i) {
        double current_time = catalogue.BusWaitTime();
        auto prev_stop = stops.at(i);
        for (int j = i + 1; j <= stops_size / 2; ++j) {
            current_time += ((catalogue.StopToStopDst(prev_stop, stops.at(j)) / 1000.0f) / bus_velocity) * 60;
            const auto edge_id = route_graph.AddEdge({ stops_vertex.at(stops.at(i)), stops_vertex.at(stops.at(j)), current_time });
            edge_stop_count_[edge_id] = EdgeInfo{ j - i, bus.bus_number };
            prev_stop = stops.at(j);
        }
    }  
    
    // Proccess all stops since middle
    for (int i = stops_size / 2; i + 1 < stops_size; ++i) {
    double current_time = catalogue.BusWaitTime();
    auto prev_stop = stops.at(i);
        for (int j = i + 1; j < stops_size; ++j) {
            current_time += ((catalogue.StopToStopDst(prev_stop, stops.at(j)) / 1000.0f) / bus_velocity) * 60;
            const auto edge_id = route_graph.AddEdge({ stops_vertex.at(stops.at(i)), stops_vertex.at(stops.at(j)), current_time });
            edge_stop_count_[edge_id] = EdgeInfo{ j - i, bus.bus_number };
            prev_stop = stops.at(j);
        }
    }
}


// Fill graph with ROUND bus trip info avoiding creating excessive edges (but create  1 excessive edge first stop -> first stop)
void SingleBusRoute::ProcessRoundBusRoute(const Bus& bus, const vector<string_view>& stops, size_t stops_size) {
    const auto bus_velocity = catalogue.BusVelocity();
    for (int i = 0; i + 1 < stops_size; ++i) {
        double current_time = catalogue.BusWaitTime();
        auto prev_stop = stops.at(i);
        for (int j = i + 1; j < stops_size; ++j) {
            current_time += ((catalogue.StopToStopDst(prev_stop, stops.at(j)) / 1000.0f) / bus_velocity) * 60;
            const auto edge_id = route_graph.AddEdge({ stops_vertex.at(stops.at(i)), stops_vertex.at(stops.at(j)), current_time });
            edge_stop_count_[edge_id] = EdgeInfo{ j - i, bus.bus_number };
            prev_stop = stops.at(j);
        }
    }
}
    

// Fill route graph with bus trips info 
void SingleBusRoute::FillRouteGraph() {
    for (const auto& bus : catalogue.GetAllBuses()) {
        const auto& stops = bus.bus_route;
        const auto stops_size = stops.size();
        if (bus.is_roundtrip) {
            ProcessRoundBusRoute(bus, stops,  stops_size);
        } else { 
            ProcessStraightBusRoute(bus, stops,  stops_size);
        }
    }
}


 void SingleBusRoute::CreateRouter() {
     router = new graph::Router<double>(route_graph);
     if (router == nullptr) {
         throw logic_error("Router didn't created");
     }
 }


// Create unordered map using for tarnsition StopId to StopName and reverse
void SingleBusRoute::SaveStopNames() {
    const auto& stops = catalogue.GetAllStops();
    for (size_t i = 0; i < stops.size(); ++i) {
        stops_vertex[stops.at(i).stop_name] = i;
        vertex_stops[i] = stops.at(i).stop_name;
    }
}

// Create minimal route between stops
optional<RouteInfo> SingleBusRoute::BuildRoute(string_view from, string_view to) const {
    return router->BuildRoute(GetIDStopByName(from),  GetIDStopByName(to));
}


size_t SingleBusRoute::GetIDStopByName(string_view stop_name) const {
    return stops_vertex.at(stop_name);
} 


string SingleBusRoute::GetStopNameByEdgeId(size_t edge_id) const {
    return string(vertex_stops.at(route_graph.GetEdge(edge_id).from));
}

double SingleBusRoute::GetEdgeWeightByEdgeId(size_t edge_id) const {
     return route_graph.GetEdge(edge_id).weight;
}

string SingleBusRoute::GetBusNameByEdgeId(size_t edge_id) const {
    return string(edge_stop_count_.at(edge_id).bus_name);
}

int SingleBusRoute::GetStopNumbByEdgeId(size_t edge_id) const {
    return edge_stop_count_.at(edge_id).stop_numb;
}

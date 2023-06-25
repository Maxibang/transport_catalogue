#include "transport_catalogue.h"

namespace transport {
   
namespace catalogue { 
    
using namespace std;

// Adds New Stop 
void TransportCatalogue::AddNewStop(const std::string& stop_name, const detail::Coordinates& coordinates) {
    stops_.push_back({stop_name, coordinates});
    stops_pointers_[stops_.back().stop_name] = &(stops_.back());
}


// Adds New Bus
void TransportCatalogue::AddNewBus(const string& bus_name, const vector<string>& stops, bool is_roundtrip = false) {
    vector<string_view> stops_view;
    stops_view.reserve(stops.size());
    
    for (const auto& stop : stops) {
        stops_view.push_back(stops_pointers_.find(stop)->first);
    }
    
    buses_.push_back({ bus_name , stops_view, is_roundtrip});
    bus_pointers_[buses_.back().bus_number] = &(buses_.back());
    
    // Fill buses_for_stop_ with stops and buses 
    for (const auto &stop_view : stops_view) {
        buses_for_stop_[stop_view].insert(buses_.back().bus_number);
    }
}


// Find Bus route
const vector<string_view>& TransportCatalogue::FindBus(const string& bus_name) const {
    if (bus_pointers_.count(bus_name) > 0) {
        return bus_pointers_.at(bus_name)->bus_route;
    }
    static vector<string_view> empty_vector;
    return empty_vector;
}
  
    
// Return ptr to definite Bus
const Bus* TransportCatalogue::FindBusPtr(const std::string_view& bus_name) const {
    if (bus_pointers_.count(bus_name) > 0) {
        return bus_pointers_.at(bus_name);
    }
    static Bus empty_bus;
    return &empty_bus;
}
    
    
// Find Stop of bus
const Stop& TransportCatalogue::FindStop(const string_view& stop_name) const {
    if(stops_pointers_.count(stop_name) > 0) {
        return *(stops_pointers_.at(stop_name)); 
    }
    static Stop empty_stop;
    return empty_stop;   
}


// Find all buses which go through single stop
const set<string_view>& TransportCatalogue::FindBusesAtStop(const string_view& stop_name) const {
    if (buses_for_stop_.count(stop_name) > 0) {
        return buses_for_stop_.at(stop_name);
    }
    static set<string_view> empty_set;
    return empty_set;
}


// Add real Distance between Stops
void TransportCatalogue::AddDstBetweenStops(const string& stop1, const int dist, const string& stop2) {
    distance_between_stops_[std::make_pair(stops_pointers_.at(stop1), stops_pointers_.at(stop2))] = dist;
}


// Return real measured distance between stops
int TransportCatalogue::StopToStopDst(const string_view& from_stop1, const string_view& to_stop2) const {
    if (distance_between_stops_.count({stops_pointers_.at(from_stop1), stops_pointers_.at(to_stop2)}) > 0) {
        return distance_between_stops_.at({stops_pointers_.at(from_stop1), stops_pointers_.at(to_stop2)});
    } 
    return distance_between_stops_.at({stops_pointers_.at(to_stop2), stops_pointers_.at(from_stop1)});
}

// Return real measured distance between stops if road between stops exists
optional<int> TransportCatalogue::RealStopsDistance(const string_view& from_stop1, const string_view& to_stop2) const {
    if (distance_between_stops_.count({ stops_pointers_.at(from_stop1), stops_pointers_.at(to_stop2) }) > 0) {
        return distance_between_stops_.at({ stops_pointers_.at(from_stop1), stops_pointers_.at(to_stop2) });
    }
    else {
        return nullopt;
    }
}
    
// Compute route lenght by GPS-coordinate and real measured distancies
detail::Distance TransportCatalogue::ComputeRouteDistance(const std::vector<std::string_view>& stops) const noexcept {
	double gps_dist{ 0 };
    int real_dist{ 0 };
    
	for (size_t i = 1; i < stops.size(); i++) {
        gps_dist += transport::detail::ComputeDistance({FindStop(stops.at(i - 1)).coordinates.lat, FindStop(stops.at(i - 1)).coordinates.lng }, 
                                    { FindStop(stops.at(i)).coordinates.lat, FindStop(stops.at(i)).coordinates.lng });
        real_dist += StopToStopDst(stops.at(i - 1), stops.at(i));
    }
	return {gps_dist, real_dist};
}
    
} // End namespace catalogue   
    
} // End namespace transport
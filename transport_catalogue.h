#pragma once

#include <set>
#include <deque>
#include <string>
#include <vector>
#include <functional>
#include <string_view>
#include <unordered_map>
#include "geo.h"
#include <optional>

/*TESTING NEW memebrs*/
#include <iostream>

// Above main-function in main.cpp you can look at namespaces structure!

namespace transport {

namespace detail {

// Single Stop
struct Stop {
    std::string stop_name = "";
    Coordinates coordinates{ 0, 0 };

    bool operator== (std::string stop_name_) const {
        return stop_name == stop_name_;
    }
    
    bool IsEmtyStop() const {
        return stop_name.size() > 0 ? false : true;
    }
    
}; // End of struct Stop


// Single Bus
struct Bus {
    std::string bus_number = "";
    std::vector<std::string_view> bus_route;
    bool is_roundtrip = false;

    bool IsEmtyBus() const {
        return bus_number.size() > 0 ? false : true;
    }

}; // End of struct Stop


// Stop to Stop Hasher
struct StopToStopHasher {
    std::size_t operator() (const std::pair<const Stop*, const Stop*>& stops_pair) const {
        return ptr_hasher(stops_pair.first) * 3 + ptr_hasher(stops_pair.second) * 3 * 3;
    }

private:
    std::hash<const void*> ptr_hasher;

}; //End of struct StopToStopHasher

} // End namespace detail 

namespace catalogue {

using Bus = detail::Bus;
using Stop = detail::Stop;
using StopToStopHasher = detail::StopToStopHasher;

class TransportCatalogue {

public:

    TransportCatalogue() = default;
    ~TransportCatalogue() {}

    // Adds New Stop    
    void AddNewStop(const std::string& stop_name, const detail::Coordinates& coordinates);

    // Adds New Bus
    void AddNewBus(const std::string& bus_name, const std::vector<std::string>& stops, bool is_roundtrip);

    // Return vector of stops names
    const std::vector<std::string_view>& FindBus(const std::string& bus_name) const;

    // Return ptr to definite Bus
    const Bus* FindBusPtr(const std::string_view& bus_name) const;

    // Find Stop of bus
    const Stop& FindStop(const std::string_view& stop_name) const;

    // Find all buses which go through single stop
    const std::set<std::string_view>& FindBusesAtStop(const std::string_view& stop_name) const;

    // Add real Distance between Stops
    void AddDstBetweenStops(const std::string& stop1, const int dist, const std::string& stop2);

    // Return real measured distance between stops
    int StopToStopDst(const std::string_view& from_stop1, const std::string_view& to_stop2) const;
    
    // Return real measured distance between stops if road exists
    std::optional<int> RealStopsDistance(const std::string_view& from_stop1, const std::string_view& to_stop2) const;

    // Compute route lenght by GPS-coordinate and real measured distancies
    detail::Distance ComputeRouteDistance(const std::vector<std::string_view>& stops) const noexcept;

    // Get access to all buses
    const std::deque<Bus>& GetAllBuses() const {
        return buses_;
    }

    // Get access to all stops
    const  std::deque<Stop>& GetAllStops() const {
        return stops_;
    }

    // Set a waiting time for every bus at stop
    void SetBusWaitTime(int minute) {
        bus_wait_time_ = minute;
    }

    // Set a bus velocity for every bus in caralogue
    void SetBusVelocity(double velocity) {
        bus_velocity_ = velocity;
    }

    int BusWaitTime() const {
        return bus_wait_time_;
    }

    int BusVelocity() const {
        return bus_velocity_;
    }
    
    // Get access to real distance between stops
    const std::unordered_map<std::pair<const Stop*, const Stop*>, int, StopToStopHasher>& RealStopDistanceData() const {
        return distance_between_stops_;
    }
    
private:

    // Variable for STOPs holding and searching
    std::deque<Stop> stops_;
    std::unordered_map<std::string_view, const Stop*> stops_pointers_;

    // Variable for BUSes holding and searching
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, const Bus*> bus_pointers_;

    // Variable for Stop X (New Request)
    std::unordered_map<std::string_view, std::set<std::string_view>> buses_for_stop_;

    // Real measured distance between Stops
    std::unordered_map<std::pair<const Stop*, const Stop*>, int, StopToStopHasher> distance_between_stops_;

    // routing_settings
    int bus_wait_time_ = 0;
    double bus_velocity_ = 0.0;



}; // End of class TransportCatalogue

} // End namespace catalogue 

} // End namespace transport
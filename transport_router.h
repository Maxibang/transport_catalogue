#pragma once

#include "graph.h"
#include "router.h"
#include <string>
#include <unordered_map>
#include "transport_catalogue.h"
#include <string_view>
#include <optional>


/*
 - Два метода удалены как лишние
 - Один метод перенесен в private
 - Логика удаленных методов перенесена внутрь существующих
 - В SingleBusRoute добавлен class Router через указатель (организовать по-другому не получилось)
 - В SingleBusRoute добавлен метод BuildRoute для получения маршрута между остановками по их названиям 
 
 Это все позволило упростить (сделать более понятным) код в месте формирования json-ответа на запрос "Route"
  
*/

struct EdgeInfo {
    int stop_numb = 0;
    std::string_view bus_name;
};

struct SingleBusRoute {
    using RouteInfo = graph::Router<double>::RouteInfo;
    using Bus = transport::detail::Bus;
    using TransportCatalogue = transport::catalogue::TransportCatalogue;
    using VertexId = size_t;
    
    SingleBusRoute(const TransportCatalogue& cat) : catalogue(cat), route_graph(catalogue.GetAllStops().size()) {
        SaveStopNames();
        FillRouteGraph();
        CreateRouter();
    }
    
    ~SingleBusRoute() {
        delete router;
    }
    
    std::optional<RouteInfo> BuildRoute(std::string_view from, std::string_view to) const;
    std::string GetBusNameByEdgeId(size_t edge_id) const;
    int GetStopNumbByEdgeId(size_t edge_id) const;   
    std::string GetStopNameByEdgeId(size_t edge_id) const;
    double GetEdgeWeightByEdgeId(size_t edge_id) const;
    
private:
    
    void SaveStopNames();
    void ProcessStraightBusRoute(const Bus& bus, const std::vector<std::string_view>& stops, size_t stops_size);
    void ProcessRoundBusRoute(const Bus& bus, const std::vector<std::string_view>& stops, size_t stops_size);
    void FillRouteGraph();
    void CreateRouter();
    size_t GetIDStopByName(std::string_view stop_name) const;

    
    const TransportCatalogue& catalogue;
    graph::DirectedWeightedGraph<double> route_graph;
    std::unordered_map<std::string_view, size_t> stops_vertex;
    std::unordered_map<size_t, std::string_view> vertex_stops;
    std::unordered_map <graph::EdgeId, EdgeInfo> edge_stop_count_;
    graph::Router<double>* router = nullptr;
};
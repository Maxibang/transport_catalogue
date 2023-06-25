#pragma once

#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"

using namespace transport;
using namespace transport::detail;
using namespace transport::detail;
using namespace transport::catalogue;
using namespace std::string_literals;
using namespace json;


using DstBetwStops = std::map<std::string, Dict>;

/* ///// **** READS DATA FROM JSON AND FILL IT INTO CATALOGUE ****///// */

// Adds info about Stop from using Dict = std::map<std::string, Node>; into catalogue
void AddStopIntoCatalogue(TransportCatalogue& catalogue, const Dict& stop_description_map, DstBetwStops& dist_to_stop);

// Adds info about Bus from using Dict = std::map<std::string, Node>; into catalogue
void AddBusIntoCatalogue(TransportCatalogue& catalogue, const Dict& bus_description_map);

// Get info about Buses and Stops from struct Document and add it into TransportCatalogue
void FillTransportCatalogue(TransportCatalogue& catalogue, const Document& document);

/* ///// **** END FILL DATA INTO CATALOGUE ****///// */


/* ///// **** READ REQUESTS FROM JSON AND FORM ANSWER ****///// */

Node ParseStopAnswer(TransportCatalogue& catalogue, const Dict& request);

Node ParseBusAnswer(const TransportCatalogue& catalogue, const Dict& request);

Node ParseSvgBusRoute(const TransportCatalogue& catalogue, const Dict& request, const RenderSettings& render_settings);

Node GetReaquestAnwer(TransportCatalogue& catalogue, const Document& document, const RenderSettings& render_settings);

Node ParseRouteAnswer(const TransportCatalogue& catalogue, const Dict& request);

/* ///// **** END FORM ANSWER ****///// */
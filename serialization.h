#pragma once

#include "json.h"
#include "json_reader.h"
#include "transport_catalogue.h"

#include <fstream>
#include <stdexcept>
#include <transport_catalogue.pb.h>
#include "map_renderer.h"


namespace serialization_catalogue {

using TransportCatalogue = transport::catalogue::TransportCatalogue;
using SerializedTransportCatalogue = transport_catalogue_serialize::TransportCatalogue;

using SerializedStop = transport_catalogue_serialize::Stop;
using SingleStop = transport::detail::Stop;

using SerializedBus = transport_catalogue_serialize::Bus;
using SingleBus = transport::detail::Bus;

using SerializedDistance = transport_catalogue_serialize::Distance;

using SerializedRenderSettings = transport_catalogue_serialize::RenderSettings;


/* ********************************* SERIALIZATION ********************************* */

// Create a SerializedStop from common Stop
SerializedStop SerializeSingleStop(const SingleStop& input_stop);

// Serialize stops from input_catalogue to serialize_catalogue
void SerializeStops(const TransportCatalogue& input_catalogue, SerializedTransportCatalogue& serialized_catalogue);

// Create a SerializedBus from common Bus
SerializedBus SerializeSingleBus(const SingleBus& input_bus);

// Serialize buses from input_catalogue to serialize_catalogue
void SerializeBuses(const TransportCatalogue& input_catalogue, SerializedTransportCatalogue& serialized_catalogue);

// Create a SerializedDistance from data
SerializedDistance SerializeSingleDistance(const std::pair<const Stop*, const Stop*> stops, int distance);

// Serialize real measured distancies between stops from input_catalogue to serialize_catalogue
void SerializeDistancies(const TransportCatalogue& input_catalogue, SerializedTransportCatalogue& serialized_catalogue);

// Serialize TransportCatalogue Data into file
void SerializeTransportCatalogue(const TransportCatalogue& catalogue, const RenderSettings& render_settings, const std::string& file);

// Serialize RenderSettings
SerializedRenderSettings GetSerializedRenderSettings(const RenderSettings& render_settings);

// Serialize routing_settings
void SerializeRoutingSettings(const TransportCatalogue& input_catalogue, SerializedTransportCatalogue& serialized_catalogue);


/* ********************************* DESERIALIZATION ********************************* */

// Add stops from serialized data from SerializedTransportCatalogue into TransportCatalogue
void DeserializeStops(const SerializedTransportCatalogue& serialized_catalogue, TransportCatalogue& catalogue);

// Add real distance between stops from SerializedTransportCatalogue into TransportCatalogue
void DeserializeDistancies(const SerializedTransportCatalogue& serialized_catalogue, TransportCatalogue& catalogue);


// Add all buses from SerializedTransportCatalogue into TransportCatalogue
void DeserializeBuses(const SerializedTransportCatalogue& serialized_catalogue, TransportCatalogue& catalogue);

//Deserialize TransportCatalogue Data from Serialized TransportCatalogue Data file
void DeserializeTransportCatalogue(const std::string file, TransportCatalogue& catalogue, RenderSettings& render_settings);

//Deserialize RenderSettings Data from Serialized TransportCatalogue Data
RenderSettings DeserializeRenderSettings(const SerializedTransportCatalogue& serialized_catalogue);

// Deserialize routing_settings
void DeserializeRoutingSettings(const SerializedTransportCatalogue& serialized_catalogue, TransportCatalogue& catalogue);


} // name space serialization_catalogue
#include "serialization.h"


namespace serialization_catalogue {


/* ********************************* SERIALIZATION ********************************* */

// Create a SerializedStop from common Stop
SerializedStop SerializeSingleStop(const SingleStop& input_stop) {
	SerializedStop serialized_stop;
	serialized_stop.set_stop_name(input_stop.stop_name);
	serialized_stop.set_latitude(input_stop.coordinates.lat);
	serialized_stop.set_longitude(input_stop.coordinates.lng);
	return serialized_stop;
}

// Serialize stops from input_catalogue to serialized_catalogue
void SerializeStops(const TransportCatalogue& input_catalogue, SerializedTransportCatalogue& serialized_catalogue) {
	for (const SingleStop& input_stop : input_catalogue.GetAllStops()) {
		*serialized_catalogue.add_stops() = std::move(SerializeSingleStop(input_stop));
	}
}


// Create a SerializedBus from common Bus
SerializedBus SerializeSingleBus(const SingleBus& input_bus) {
	SerializedBus serialized_bus;
	serialized_bus.set_bus_number(input_bus.bus_number);
	serialized_bus.set_roundtrip(input_bus.is_roundtrip);
	for (const auto stop : input_bus.bus_route) {
		*serialized_bus.add_stops_at_route() = std::move(std::string(stop));
	}
	return serialized_bus;
}

// Serialize buses from input_catalogue to serialize_catalogue
void SerializeBuses(const TransportCatalogue& input_catalogue, SerializedTransportCatalogue& serialized_catalogue) {
	for (const SingleBus& single_bus : input_catalogue.GetAllBuses()) {
		*serialized_catalogue.add_buses() = std::move(SerializeSingleBus(single_bus));
	}
}


// Create a SerializedDistance from data
SerializedDistance SerializeSingleDistance(const std::pair<const Stop*, const Stop*> stops, int distance) {
	SerializedDistance serialized_distance;
	serialized_distance.set_from_stop(stops.first->stop_name);
	serialized_distance.set_to_stop(stops.second->stop_name);
	serialized_distance.set_distance(distance);
	return serialized_distance;
}

// Serialize real measured distancies between stops from input_catalogue to serialize_catalogue
void SerializeDistancies(const TransportCatalogue& input_catalogue, SerializedTransportCatalogue& serialized_catalogue) {
	for (const auto [stops, distance] : input_catalogue.RealStopDistanceData()) {
		*serialized_catalogue.add_distancies() = SerializeSingleDistance(stops, distance);
	}
}


// Serialize TransportCatalogue Data into file
void SerializeTransportCatalogue(const TransportCatalogue& catalogue, const RenderSettings& render_settings, const std::string& file) {
	std::ofstream out_file(file, std::ios::binary);

	if (!out_file.is_open()) {
		throw std::logic_error("Can't open file");
	}

	SerializedTransportCatalogue serialized_cataloge;

	// SERIALIZE ALL STOPS 
	SerializeStops(catalogue, serialized_cataloge);

	// SERIALIZE ALL BUSES
	SerializeBuses(catalogue, serialized_cataloge);

	// SERIALIZE ALL DISTANCIES
	SerializeDistancies(catalogue, serialized_cataloge);
    
    // SERIALIZE RenderSettings
    *serialized_cataloge.mutable_render_settings() = std::move(GetSerializedRenderSettings(render_settings));

	// SERIALIZE routing_settings
	SerializeRoutingSettings(catalogue, serialized_cataloge);
        
	//Serialize serialized_cataloge into outfile
	serialized_cataloge.SerializeToOstream(&out_file);
}

    
// Serialize RenderSettings
SerializedRenderSettings GetSerializedRenderSettings(const RenderSettings& render_settings) {
    
    SerializedRenderSettings settings;
    
    settings.set_width(render_settings.width);
    settings.set_height(render_settings.height);

    settings.set_padding(render_settings.padding);
    
    settings.set_line_width(render_settings.line_width);
    settings.set_stop_radius(render_settings.stop_radius);

    settings.set_bus_label_font_size(render_settings.bus_label_font_size);
    settings.mutable_bus_label_offset()->set_x(render_settings.bus_label_offset.x);
    settings.mutable_bus_label_offset()->set_y(render_settings.bus_label_offset.y);
    
    settings.set_stop_label_font_size(render_settings.stop_label_font_size);
    settings.mutable_stop_label_offset()->set_x(render_settings.stop_label_offset.x);
    settings.mutable_stop_label_offset()->set_y(render_settings.stop_label_offset.y);
    
    settings.set_underlayer_color(render_settings.underlayer_color);
    settings.set_underlayer_width(render_settings.underlayer_width);
    
    for (size_t i = 0; i < render_settings.color_palette.size(); ++i) {
        *settings.add_color_palette() = render_settings.color_palette.at(i);
    }
    
    return settings;
}


// Serialize routing_settings
void SerializeRoutingSettings(const TransportCatalogue& input_catalogue, SerializedTransportCatalogue& serialized_catalogue) {
	serialized_catalogue.set_bus_wait_time(input_catalogue.BusWaitTime());
	serialized_catalogue.set_bus_velocity(input_catalogue.BusVelocity());

}


/* ********************************* DESERIALIZATION ********************************* */

// Add stops SerializedTransportCatalogue into TransportCatalogue
void DeserializeStops(const SerializedTransportCatalogue& serialized_catalogue, TransportCatalogue& catalogue) {
	const auto size = serialized_catalogue.stops_size();
	for (size_t i = 0; i < size; ++i) {
		catalogue.AddNewStop(serialized_catalogue.stops(i).stop_name(), { serialized_catalogue.stops(i).latitude(), serialized_catalogue.stops(i).longitude() });
	}
}


// Add real distance between stops from SerializedTransportCatalogue into TransportCatalogue
void DeserializeDistancies(const SerializedTransportCatalogue& serialized_catalogue, TransportCatalogue& catalogue) {
	const auto size = serialized_catalogue.distancies_size();
	for (size_t i = 0; i < size; ++i) {
		catalogue.AddDstBetweenStops(serialized_catalogue.distancies(i).from_stop(), serialized_catalogue.distancies(i).distance(), serialized_catalogue.distancies(i).to_stop());
	}
}

// Add all buses from SerializedTransportCatalogue into TransportCatalogue
void DeserializeBuses(const SerializedTransportCatalogue& serialized_catalogue, TransportCatalogue& catalogue) {
	const auto bus_count = serialized_catalogue.buses_size();
	for (size_t i = 0; i < bus_count; ++i) {
		std::vector<std::string> stops;
		const auto stop_count = serialized_catalogue.buses(i).stops_at_route_size();
		stops.reserve(stop_count);

		for (size_t j = 0; j < stop_count; ++j) {
			stops.push_back(serialized_catalogue.buses(i).stops_at_route(j));
		}
	catalogue.AddNewBus(serialized_catalogue.buses(i).bus_number(), stops, serialized_catalogue.buses(i).roundtrip());
	}
}


void DeserializeTransportCatalogue(const std::string file, TransportCatalogue& catalogue, RenderSettings& render_settings) {

	std::ifstream in_file(file, std::ios::binary);

	if (!in_file.is_open()) {
		throw std::logic_error("Can't open file");
	}

	transport_catalogue_serialize::TransportCatalogue serialized_catalogue;
	serialized_catalogue.ParseFromIstream(&in_file);

	// Add stops from SerializedTransportCatalogue into TransportCatalogue
	DeserializeStops(serialized_catalogue, catalogue);

	// Add real distance between stops from SerializedTransportCatalogue into TransportCatalogue
	DeserializeDistancies(serialized_catalogue, catalogue);

	// Add all buses from SerializedTransportCatalogue into TransportCatalogue
	DeserializeBuses(serialized_catalogue, catalogue);
    
    render_settings = std::move(DeserializeRenderSettings(serialized_catalogue));

	DeserializeRoutingSettings(serialized_catalogue, catalogue);
}
    
    
//Deserialize RenderSettings Data from Serialized TransportCatalogue Data
RenderSettings DeserializeRenderSettings(const SerializedTransportCatalogue& serialized_catalogue) {
    
    RenderSettings render_settings;
        
    render_settings.width = serialized_catalogue.render_settings().width();
    render_settings.height = serialized_catalogue.render_settings().height();
    render_settings.padding = serialized_catalogue.render_settings().padding();
    render_settings.line_width = serialized_catalogue.render_settings().line_width();
    render_settings.stop_radius = serialized_catalogue.render_settings().stop_radius();
    
    render_settings.bus_label_font_size = serialized_catalogue.render_settings().bus_label_font_size();
    render_settings.bus_label_offset.x = serialized_catalogue.render_settings().bus_label_offset().x();
    render_settings.bus_label_offset.y = serialized_catalogue.render_settings().bus_label_offset().y();

    render_settings.stop_label_font_size = serialized_catalogue.render_settings().stop_label_font_size();
    render_settings.stop_label_offset.x = serialized_catalogue.render_settings().stop_label_offset().x();
    render_settings.stop_label_offset.y = serialized_catalogue.render_settings().stop_label_offset().y();
    
    render_settings.underlayer_color = serialized_catalogue.render_settings().underlayer_color();
    render_settings.underlayer_width = serialized_catalogue.render_settings().underlayer_width(); 
    
    const auto size = serialized_catalogue.render_settings().color_palette().size();
    render_settings.color_palette.reserve(size);
    
    // Fill color_palette
    for (size_t i = 0; i < size; ++i) {
        render_settings.color_palette.push_back(serialized_catalogue.render_settings().color_palette(i));
    }
    return render_settings;
}


// Deserialize routing_settings
void DeserializeRoutingSettings(const SerializedTransportCatalogue& serialized_catalogue, TransportCatalogue& catalogue) {
	catalogue.SetBusWaitTime(serialized_catalogue.bus_wait_time());
	catalogue.SetBusVelocity(serialized_catalogue.bus_velocity());
}

} // namespace serialization_catalogue
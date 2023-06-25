#include "request_handler.h"

namespace serialization_catalogue {


	

/* ********************************* DATABASE PROCESSING ********************************* */

// Read data from json into TransportCatalogue and serialize it
void MakeBase(std::istream& input) {
	TransportCatalogue catalogue;
	auto input_data_document_ = json::Load(input);

	FillTransportCatalogue(catalogue, input_data_document_);
    const RenderSettings render_settings = SaveRenderSettings(input_data_document_);

	// Get file_name for serialization
	const auto file_name = input_data_document_.GetRoot().AsMap().at("serialization_settings"s).AsMap().at("file").AsString();

	//Serialize catalogue into file_name
	SerializeTransportCatalogue(catalogue, render_settings, file_name);
}

// Deserialize data and process requests
void ProcessRequests(std::ostream& out, std::istream& input) {
	const auto input_data_document_ = json::Load(input);

	// Get file_name for serialization
	const auto file_name = input_data_document_.GetRoot().AsMap().at("serialization_settings"s).AsMap().at("file").AsString();

	TransportCatalogue catalogue;
    RenderSettings render_settings;
    
	// Deserialize catalogue from file_name
	DeserializeTransportCatalogue(file_name, catalogue, render_settings);
	
	

	// Get answers
	json::Document output_data_document_(GetReaquestAnwer(catalogue, input_data_document_, render_settings));

	// Print answers into out
	json::Print(output_data_document_, out);
}


} // namespace serialization_catalogue
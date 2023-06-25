#pragma once

#include "json.h"
#include "json_reader.h"
#include "transport_catalogue.h"

#include <fstream>
#include <stdexcept>
#include <transport_catalogue.pb.h>
#include "map_renderer.h"
#include "serialization.h"



namespace serialization_catalogue {
    
/* ********************************* DATABASE PROCESSING ********************************* */

// Read data from json into TransportCatalogue and serialize it
void MakeBase(std::istream& input = std::cin);

// Deserialize data and process requests
void ProcessRequests(std::ostream& out = std::cout, std::istream& input = std::cin);

} // namespace serialization_catalogue
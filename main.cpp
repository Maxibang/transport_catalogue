#include <fstream>
#include <iostream>
#include <string_view>
#include "request_handler.h"

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    
  
    if (argc != 2) {
        PrintUsage();
        return 1;
    }
 

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        // make base here
        serialization_catalogue::MakeBase();

    } else if (mode == "process_requests"sv) {
        // process requests here
        serialization_catalogue::ProcessRequests();

    } else {
        PrintUsage();
        return 1;
    }
}

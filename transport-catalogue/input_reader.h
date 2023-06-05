#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <execution>
#include "transport_catalogue.h"

namespace transport_catalogue {
    namespace input_parser_reader {
        using namespace transport_catalogue::catalogue;
        namespace details {
            std::string_view Trim(std::string_view sv);
            std::vector<std::string_view> Split(std::string_view s, char sep);
        }
        
        using namespace details;
        
        std::vector<std::string> ReadQuerys(std::istream& input);

        void ParseAddBusQuery(std::string_view raw_query, TransportCatalogue& tc);

        void ParseStopQueryCoordinates(std::string_view raw_query, TransportCatalogue& tc);
        
        void ParseStopQueryDistances(std::string_view raw_query, TransportCatalogue& tc);
        
        std::vector<std::pair<std::string_view, int>> ParseDistances(std::vector<std::string_view> raw_distances);
        
        void ParseQuerys(std::vector<std::string> raw_querys, TransportCatalogue& tc);   
    }   
}



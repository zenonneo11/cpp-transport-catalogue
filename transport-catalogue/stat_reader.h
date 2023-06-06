#pragma once
#include <string>
#include <algorithm>
#include <set>
#include "input_reader.h"
#include "geo.h"

namespace transport_catalogue {
    namespace stat_reader_printer {
        using namespace transport_catalogue::input_parser_reader;
        
        enum class ResponseType {
            BUS_RESPONSE,
            STOP_RESPONSE,
        };

        struct Response {
            bool is_exist_;
            std::string name_;
            size_t total_stops_count_;
            size_t unique_stops_count_;
            double route_lenght_;
            int real_route_lenght_;
            ResponseType r_type;
            std::set<std::string_view> buses_for_stop;
        };



        void PrintBusResponce(const Response& response, std::ostream& out);

        void PrintStopResponce(const Response& response, std::ostream& out);

        std::vector<Response> ParseStatQuerys(std::vector<std::string> raw_querys, TransportCatalogue& tc);
    
        void StatHandler(std::vector<std::string> raw_querys, TransportCatalogue& tc);
    }
    
}


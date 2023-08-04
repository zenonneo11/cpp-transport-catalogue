#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include "transport_catalogue.h"
#include "request_handler.h"
#include "json.h"
#include "map_renderer.h"

namespace transport_catalogue {
    namespace input_parser_reader {
        using namespace json;
        using namespace transport_catalogue::catalogue;

        void ParseBaseRequests(const Document& doc, TransportCatalogue& tc);

        void ParseStopQueryCoordinates(const Node& request, TransportCatalogue& tc);

        void ParseStopQueryDistances(const Node& request, TransportCatalogue& tc);

        void ParseAddBusQuery(const Node& request, TransportCatalogue& tc);

        void ParseRenderSettings(const json::Document& doc, renderer::MapRenderer& mr);

        void ParseRoutingSettings(const json::Document& doc);

        Document HandleStatRequests(const Document& json_req, const transport_catalogue::RequestHandler& req_hndlr);

        std::vector<Node> ParseStatRequests(const Array& stat_requests, const RequestHandler& req_hndlr);
    }
}



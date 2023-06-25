#include "json_reader.h"
#include <string_view>
#include <unordered_set>


namespace transport_catalogue {
    namespace input_parser_reader {
        using namespace transport_catalogue::catalogue;

        void ParseAddBusQuery(const Node& request, TransportCatalogue& tc) {
            const Dict& request_data = request.AsMap();
            if (request_data.at("type").AsString() == "Bus") {
                Bus bus;
                bus.name = request_data.at("name").AsString();
                bus.is_loop = request_data.at("is_roundtrip").AsBool();
                const auto& stops = request_data.at("stops").AsArray();

                for (const auto& stop : stops) {
                    bus.stops.push_back(tc.GetStop(stop.AsString()));
                }
                if (!bus.is_loop) {

                    for (auto begin = next(stops.rbegin()), end = stops.rend(); begin != end; ++begin) {
                        bus.stops.push_back(tc.GetStop((*begin).AsString()));
                    }
                }
                tc.AddBus(std::move(bus));
            }
            else
            {
                return;
            }
        }
        svg::Color ParceColor(const Node& n) {
            if (n.IsString())
                return n.AsString();
            else {
                if (n.AsArray().size() == 3) {
                    uint8_t r = n.AsArray().at(0).AsInt();
                    uint8_t g = n.AsArray().at(1).AsInt();
                    uint8_t b = n.AsArray().at(2).AsInt();
                    return svg::Rgb{ r,g,b };
                }
                else {
                    uint8_t r = n.AsArray().at(0).AsInt();
                    uint8_t g = n.AsArray().at(1).AsInt();
                    uint8_t b = n.AsArray().at(2).AsInt();
                    double opacity = n.AsArray().at(3).AsDouble();
                    return svg::Rgba{ r,g,b, opacity };
                }
            }
        }
        void ParseRenderSettings(const json::Document& doc, renderer::MapRenderer& mr) {
                const json::Dict& v_props = doc.GetRoot().AsMap().at("render_settings").AsMap();
                renderer::MapRenderer::VisualiseProps props;
                props.width = v_props.at("width").AsDouble();
                props.height = v_props.at("height").AsDouble();
                props.padding = v_props.at("padding").AsDouble();
                props.line_width = v_props.at("line_width").AsDouble();
                props.stop_radius = v_props.at("stop_radius").AsDouble();
                props.bus_label_font_size = v_props.at("bus_label_font_size").AsInt();
                const auto& label_offset_bus = v_props.at("bus_label_offset").AsArray();
                props.bus_label_offset = { label_offset_bus[0].AsDouble(), label_offset_bus[1].AsDouble() };
                props.stop_label_font_size = v_props.at("stop_label_font_size").AsInt();
                const auto& label_offset_stop = v_props.at("stop_label_offset").AsArray();
                props.stop_label_offset = { label_offset_stop[0].AsDouble(), label_offset_stop[1].AsDouble() };
                props.underlayer_color = ParceColor(v_props.at("underlayer_color"));
                props.underlayer_width = v_props.at("underlayer_width").AsDouble();
                auto colors = v_props.at("color_palette").AsArray();
                props.color_palette.resize(colors.size());
                std::transform(colors.begin(), colors.end(), props.color_palette.begin(), [](const Node& node) {
                    return ParceColor(node); });

                mr.props = props;
        }

        void ParseStopQueryCoordinates(const Node& request, TransportCatalogue& tc) {
            const Dict& request_data = request.AsMap();
            if (request_data.at("type").AsString() == "Stop") {
                Stop stop;
                stop.name = request_data.at("name").AsString();
                stop.coordinates.lat = request_data.at("latitude").AsDouble();
                stop.coordinates.lng = request_data.at("longitude").AsDouble();
                tc.AddStop(std::move(stop));
            }
            else {
                return;
            }
        }

        void ParseStopQueryDistances(const Node& request, TransportCatalogue& tc) {
            const Dict& request_data = request.AsMap();
            if (request_data.at("type").AsString() == "Stop") {
                std::string_view stop_name = request_data.at("name").AsString();
                const Dict& stop_distances = request_data.at("road_distances").AsMap();
                for (const auto& [other_stop, distance] : stop_distances) {
                    tc.SetDistance(tc.GetStop(stop_name), tc.GetStop(other_stop), distance.AsInt());
                }
            }
            else {
                return;
            }
        }

        void ParseBaseRequests(const Document& doc, TransportCatalogue& tc) {
            const Array& requests = doc.GetRoot().AsMap().at("base_requests").AsArray();
            std::for_each(requests.begin(), requests.end(), [&](const Node& request) {ParseStopQueryCoordinates(request, tc); });
            std::for_each(requests.begin(), requests.end(), [&](const Node& request) {ParseStopQueryDistances(request, tc); });
            std::for_each(requests.begin(), requests.end(), [&](const Node& request) {ParseAddBusQuery(request, tc); });
        }


        std::vector<Node> ParseStatRequests(const Array& stat_requests, const RequestHandler& req_hndlr) {
            std::vector<Node> res;
            res.reserve(stat_requests.size());

            for (const auto& request : stat_requests) {

                if (request.AsMap().at("type").AsString() == "Bus") {
                    int id = request.AsMap().at("id").AsInt();
                    std::string bus_name = request.AsMap().at("name").AsString();

                    if (auto resp = req_hndlr.GetBusStat(bus_name)) {
                        res.push_back(
                            Dict{
                                {"curvature",resp->curvature },
                                {"request_id", id},
                                {"route_length", resp->real_route_lenght},
                                {"stop_count", resp->bus_stops_count},
                                {"unique_stop_count", resp->unique_stops_count}
                            });
                    }
                    else {
                        res.push_back(
                            Dict{
                                {"request_id"s, id},
                                {"error_message"s, "not found"s}
                            });
                    }
                }
                else if (request.AsMap().at("type").AsString() == "Stop") {
                    auto id = request.AsMap().at("id").AsInt();
                    auto stop_name = request.AsMap().at("name").AsString();

                    if (auto resp = req_hndlr.GetBusesByStop(stop_name)) {
                        Array arr(resp->size());
                        std::transform(resp->begin(), resp->end(), arr.begin(), [](std::string_view sv) {return std::string(sv); });
                        res.push_back(
                            Dict{
                                {"buses" , arr },
                                {"request_id", id}
                            });
                    }
                    else
                    {
                        res.push_back(
                            Dict{
                                {"request_id"s, id},
                                {"error_message"s, "not found"s}
                            });
                    }
                }
                else if (request.AsMap().at("type").AsString() == "Map") {
                    auto id = request.AsMap().at("id").AsInt();
                    svg::Document d = req_hndlr.RenderMap();
                    std::stringstream out;
                    d.Render(out);
                    //d.Render(cout);
                    res.push_back(
                        Dict{
                            {"map", out.str()},
                            {"request_id", id}
                        }
                    );
                }
            }
            return res;
        }

        Document HandleStatRequests(const Document& json_req, const RequestHandler& req_hndlr) {
            const Array& stat_requests = json_req.GetRoot().AsMap().at("stat_requests").AsArray();
            return Document(ParseStatRequests(stat_requests, req_hndlr));
        }
    }
}



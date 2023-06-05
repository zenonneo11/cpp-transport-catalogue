#include "stat_reader.h"


namespace transport_catalogue {
    namespace stat_reader_printer {
        using namespace transport_catalogue::input_parser_reader;

        void PrintBusResponce(const Response& response, std::ostream& out) {
            if (response.is_exist_)
                out << "Bus " << response.name_ << ": " << response.total_stops_count_ << " stops on route, "
                << response.unique_stops_count_ << " unique stops, " << response.real_route_lenght_
                << " route length, " << response.real_route_lenght_ / response.route_lenght_ << " curvature" << std::endl;
            else
                out << "Bus " << response.name_ << ": not found" << std::endl;
        }

        void PrintStopResponce(const Response& response, std::ostream& out) {

            if (response.is_exist_ && !response.buses_for_stop.empty()) {
                out << "Stop " << response.name_ << ": buses";
                for (auto bus : response.buses_for_stop)
                    out << ' ' << bus;
                out << std::endl;
            }
            else if (response.is_exist_ && response.buses_for_stop.empty()) {
                out << "Stop " << response.name_ << ": no buses" << std::endl;
            }
            else {
                out << "Stop " << response.name_ << ": not found" << std::endl;
            }
        }

        std::vector<Response> ParseStatQuerys(std::vector<std::string> raw_querys, TransportCatalogue& tc) {
            std::vector<Response> res;
            res.reserve(raw_querys.size());
            for (std::string_view raw_query : raw_querys) {
                if (raw_query.substr(0, 3) == "Bus") {
                    raw_query.remove_prefix(std::string_view("Bus").size());
                    auto bus_name = Trim(raw_query);
                    auto bus = tc.GetBus(bus_name);
                    size_t unique_stops_count = 0;
                    double route_lenght = 0;
                    int real_route_lenght = 0;
                    if (bus.is_exists) {
                        std::set<const Stop*> s(bus.stops.begin(), bus.stops.end());
                        unique_stops_count = s.size();
                        route_lenght = transform_reduce(bus.stops.begin(), std::prev(bus.stops.end()), std::next(bus.stops.begin()), 0.0, std::plus{},
                            [](const Stop* s1, const Stop* s2) {return ComputeDistance(s1->coordinates, s2->coordinates); });
                        real_route_lenght = transform_reduce(bus.stops.begin(), std::prev(bus.stops.end()), std::next(bus.stops.begin()), 0.0, std::plus{},
                            [&](const Stop* s1, const Stop* s2) {return tc.GetDistance(s1, s2); });

                    }
                    res.push_back({ bus.is_exists , bus.name, bus.is_loop ? bus.stops.size() - 1 : bus.stops.size(), unique_stops_count, route_lenght, real_route_lenght, ResponseType::BUS_RESPONSE, {} });
                }
                else if (raw_query.substr(0, 4) == "Stop") {
                    raw_query.remove_prefix(std::string_view("Stop").size());
                    auto stop_name = Trim(raw_query);
                    auto [is_exist, stops] = tc.GetBusesForStop(stop_name);
                    res.push_back({ is_exist , std::string(stop_name), 0, 0, 0, 0, ResponseType::STOP_RESPONSE, stops });
                }
            }
            return res;
        }
    
    }
    
}


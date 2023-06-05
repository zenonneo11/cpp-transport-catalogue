#include "input_reader.h"
#include "transport_catalogue.h"

namespace transport_catalogue {
    namespace input_parser_reader {
        using namespace transport_catalogue::catalogue;
        namespace details {
            std::string_view Trim(std::string_view sv) {
                while (!sv.empty() && isspace(sv.front())) {
                    sv.remove_prefix(1);
                }
                while (!sv.empty() && isspace(sv.back())) {
                    sv.remove_suffix(1);
                }
                return sv;
            }

            std::vector<std::string_view> Split(std::string_view s, char sep) {
                std::vector<std::string_view> result;
                while (!s.empty()) {
                    size_t pos = s.find(sep);
                    result.push_back(s.substr(0, pos));
                    s.remove_prefix(pos != s.npos ? pos + 1 : s.size());
                }
                return result;
            }            
        }
        
        using namespace details;
        
        std::vector<std::string> ReadQuerys(std::istream& input) {
            int query_count;
            input >> query_count;
            std::string dummy;
            getline(input, dummy);
            std::vector<std::string> raw_querys(query_count);
            for (auto& query : raw_querys) {
                std::getline(input, query);
            }
            return raw_querys;
        }


        void ParseAddBusQuery(std::string_view raw_query, TransportCatalogue& tc) {
            if (raw_query.substr(0, 3) == "Bus") {
                Bus bus;
                raw_query.remove_prefix(std::string_view("Bus").size());
                auto pos = raw_query.find(':');
                bus.name = std::string{ Trim(raw_query.substr(0, pos)) };
                raw_query.remove_prefix(pos + 1);

                if (raw_query.find('>') != raw_query.npos) {
                    bus.is_loop = false;
                    auto stops = Split(raw_query, '>');
                    std::transform(stops.begin(), stops.end(), stops.begin(), [](std::string_view sv) {return details::Trim(sv); });

                    for (std::string_view stop : stops) {
                        bus.stops.push_back(tc.GetStop(stop));
                    }
                    tc.AddBus(std::move(bus));
                }
                else if (raw_query.find('-') != raw_query.npos) {
                    bus.is_loop = true;
                    auto stops = details::Split(raw_query, '-');
                    std::transform(stops.begin(), stops.end(), stops.begin(), [](std::string_view sv) {return Trim(sv); });

                    for (std::string_view stop : stops) {
                        bus.stops.push_back(tc.GetStop(stop));
                    }

                    for (auto begin = stops.rbegin(), end = stops.rend(); begin != end; ++begin) {
                        bus.stops.push_back(tc.GetStop(*begin));
                    }
                    tc.AddBus(std::move(bus));
                }
            }
            else
            {
                return;
            }
        }

        void ParseStopQueryCoordinates(std::string_view raw_query, TransportCatalogue& tc) {
            if (raw_query.substr(0, 4) == "Stop") {
                raw_query.remove_prefix(std::string_view("Stop").size());
                auto pos = raw_query.find(':');
                Stop stop;
                stop.name = std::string{ Trim(raw_query.substr(0, pos)) };
                raw_query.remove_prefix(pos + 1);
                pos = raw_query.find(',');

                auto lat = raw_query.substr(0, pos);
                auto lng = raw_query.substr(pos + 1, raw_query.npos);

                stop.coordinates.lat = stod(std::string(lat));
                stop.coordinates.lng = stod(std::string(lng));

                tc.AddStop(std::move(stop));
            }
            else {
                return;
            }
        }

        std::vector<std::pair<std::string_view, int>> ParseDistances(std::vector<std::string_view> raw_distances) {
            std::vector<std::pair<std::string_view, int>> res;
            for (auto distance_record : raw_distances) {
                auto distance = std::stoi(std::string(distance_record));
                auto to_ = distance_record.find("to");
                distance_record.remove_prefix(to_ + 2);
                res.push_back({ Trim(distance_record), distance });
            }

            return res;
        }

        void ParseStopQueryDistances(std::string_view raw_query, TransportCatalogue& tc) {
            if (raw_query.substr(0, 4) == "Stop") {
                raw_query.remove_prefix(std::string_view("Stop").size());
                auto pos = raw_query.find(':');
                Stop stop;
                stop.name = std::string{ Trim(raw_query.substr(0, pos)) };
                raw_query.remove_prefix(pos + 1);
                auto raw_distanses = Split(raw_query, ',');
                const int elements_to_skip = 2;
                auto stop_distance = ParseDistances({ std::next(raw_distanses.begin(), elements_to_skip), raw_distanses.end() });
                for (auto&& [other_stop, distance] : stop_distance) {
                    tc.SetDistance(tc.GetStop(stop.name), tc.GetStop(other_stop), distance);
                }
            }
            else {
                return;
            }
        }

        void ParseQuerys(std::vector<std::string> raw_querys, TransportCatalogue& tc) {
            std::for_each(raw_querys.begin(), raw_querys.end(), [&](std::string_view sv) {ParseStopQueryCoordinates(sv, tc); });
            std::for_each(raw_querys.begin(), raw_querys.end(), [&](std::string_view sv) {ParseStopQueryDistances(sv, tc); });
            std::for_each(raw_querys.begin(), raw_querys.end(), [&](std::string_view sv) {ParseAddBusQuery(sv, tc); });
        }   
    }   
}



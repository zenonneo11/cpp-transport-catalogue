
#include "transport_catalogue.h"

namespace transport_catalogue {
    namespace catalogue{
        
        void TransportCatalogue::AddBus(Bus bus) {
            buses_.push_back(std::move(bus));
            bus_index_[buses_.back().name] = &buses_.back();
            for (auto stop : buses_.back().stops) {
                stop_to_buses[stop->name].insert(buses_.back().name);
            }
        }

        void TransportCatalogue::AddStop(Stop stop) {
            if (!stop_index_.count(stop.name)) {
                stops_.push_back(std::move(stop));
                stop_index_[stops_.back().name] = &stops_.back();
            }
        }
    
        const Bus& TransportCatalogue::GetBus(std::string_view bus) const {
            if (bus_index_.count(bus)) {
                return *(bus_index_.at(bus));
            }
            else {
                static Bus bus_;
                bus_.is_exists = false;
                bus_.name = bus;
                return bus_;
            }
        }

        const Stop* TransportCatalogue::GetStop(std::string_view stop) const {
            return stop_index_.at(stop);
        }

        std::pair<bool, std::set<std::string_view>> TransportCatalogue::GetBusesForStop(std::string_view stop) const {
            if (stop_to_buses.count(stop)) {
                return { true, stop_to_buses.at(stop) };
            }
            else if (stop_index_.count(stop)) {
                return { true, {} };
            }
            else
            {
                return { false, {} };
            }
        }

        void TransportCatalogue::SetDistance(const Stop* s1, const Stop* s2, int distance) {
            stops_distance[{s1, s2}] = distance;
        }

        int TransportCatalogue::GetDistance(const Stop* s1, const Stop* s2) const {
            if (stops_distance.count({ s1, s2 })) {
                return stops_distance.at({ s1, s2 });
            }
            else if (stops_distance.count({ s2, s1 })) {
                return stops_distance.at({ s2, s1 });
            }
            else {
                return 0;
            }
        }                        
    }    
}



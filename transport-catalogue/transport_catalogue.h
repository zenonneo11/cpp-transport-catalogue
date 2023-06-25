#pragma once

#include <deque>
#include <vector>
#include <set>
#include <string>
#include <unordered_map>
#include "geo.h"
#include "domain.h"
namespace transport_catalogue {
    namespace catalogue{
        using namespace geo;

        class TransportCatalogue {

            public:
            void AddBus(Bus&& bus);

            void AddStop(Stop&& stop);

            const Bus& GetBus(std::string_view bus) const;

            const Stop* GetStop(std::string_view stop) const;

            const std::set<std::string_view>& GetBusesForStop(std::string_view stop) const;
            
            void SetDistance(const Stop* s1, const Stop* s2, int distance);

            int GetDistance(const Stop* s1, const Stop* s2) const;

            std::vector<const Bus*> GetAllBuses() const;


            private:
            std::deque<Stop> stops_;
            std::deque<Bus> buses_;

            std::unordered_map<std::string_view, const Bus*> bus_index_;
            std::unordered_map<std::string_view, const Stop*> stop_index_;

            std::unordered_map<std::string_view, std::set<std::string_view>> stop_to_buses;
            
            struct StopDistanceHasher {
                size_t operator()(const std::pair<const Stop*, const Stop*>& p) const {
                    return hasher(p.first) + hasher(p.second) * 17;
                }
                std::hash<const void*> hasher;
            };            
            std::unordered_map<std::pair<const Stop*, const Stop*>, int, StopDistanceHasher> stops_distance;                       
        };            
    }    
}



#pragma once
#include <string>
#include <algorithm>
#include <set>
#include <map>
#include <numeric>
#include "geo.h"
#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "domain.h"
#include <unordered_set>


namespace transport_catalogue {

    struct BusStat {
        int bus_stops_count;
        int unique_stops_count;
        int real_route_lenght;
        double curvature;
    };

    class RequestHandler {
    public:
        // MapRenderer понадобится в следующей части итогового проекта
        RequestHandler(const catalogue::TransportCatalogue& db, const renderer::MapRenderer& renderer) :db_(db), renderer_(renderer) {}

        // Возвращает информацию о маршруте (запрос Bus)
        std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const {
            const Bus& bus = db_.GetBus(bus_name);
            if (bus.is_exists) {
                int bus_stops_count = bus.stops.size();
                std::unordered_set<const Stop*> s(bus.stops.begin(), bus.stops.end());
                int unique_stops_count = s.size();
                double route_lenght = std::transform_reduce(bus.stops.begin(), std::prev(bus.stops.end()), std::next(bus.stops.begin()), 0.0, std::plus{},
                    [](const Stop* s1, const Stop* s2) {return ComputeDistance(s1->coordinates, s2->coordinates); });
                int real_route_lenght = std::transform_reduce(bus.stops.begin(), std::prev(bus.stops.end()), std::next(bus.stops.begin()), 0.0, std::plus{},
                    [&](const Stop* s1, const Stop* s2) {return db_.GetDistance(s1, s2); });
                return { {bus_stops_count, unique_stops_count, real_route_lenght , real_route_lenght / route_lenght} };

            }
            else return {};
        }

        // Возвращает маршруты, проходящие через остановку (запрос Stop)
        std::optional<std::set<std::string_view>> GetBusesByStop(const std::string_view& stop_name) const {
            const Stop* stop = db_.GetStop(stop_name);
            if (stop->is_exists) {
                return { db_.GetBusesForStop(stop_name) };
            }
            else
            {
                return {};
            }
        }

        svg::Document RenderMap() const {

            std::vector<const Bus*> all_buses = db_.GetAllBuses();
            std::vector<std::vector<const Stop*>> buses_stops;
            buses_stops.reserve(all_buses.size());

            for (const Bus* bus : all_buses) {
                buses_stops.push_back(bus->stops);
            }

            std::vector<geo::Coordinates> stops_coords;
            for (const auto& stops : buses_stops) {
                for (const auto& stop : stops) {
                    stops_coords.push_back(stop->coordinates);
                }
            }

            const renderer::SphereProjector proj{stops_coords.begin(), stops_coords.end(), renderer_.props.width, renderer_.props.height, renderer_.props.padding};

            svg::Document map;

            renderer_.RenderLines(map, buses_stops, proj);
            renderer_.RenderBusNames(map, all_buses, proj);
            renderer_.RenderStops(map, all_buses, proj);
            renderer_.RenderStopNames(map, all_buses, proj);

            return map;
        }

    private:
        // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
        const catalogue::TransportCatalogue& db_;
        const renderer::MapRenderer& renderer_;
    };
}



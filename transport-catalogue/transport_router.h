#pragma once

#include <optional>
#include <vector>
#include <string>
#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"
#include <unordered_map>

namespace transport_catalogue {
	using namespace transport_catalogue::catalogue;
	using namespace graph;

	class TransportRouter {
        
    private:
        inline static int bus_wait_time_ = 0 ;
        inline static double bus_velocity_ = 0;
        
        std::unordered_map<graph::VertexId, const Stop*> id_to_stop;
		std::unordered_map<const void*, graph::VertexId> stop_to_vertex_id;
		graph::DirectedWeightedGraph<double> graph_;
		graph::Router<double> router_;
        
        graph::DirectedWeightedGraph<double> BuildGraph(const TransportCatalogue& tc);
        
        VertexId GetVertexId(const Stop* stop);
        
        const Stop* GetStopByVertexID(VertexId id) const;
        
    public:        
        struct Edge {
			std::string bus;
			std::string from;
			std::string to;
			double weight;
			int stops_count;
		};

		struct RouteInfo {
			double weight;
			std::vector<Edge> edges;		
		};
        
        static void SetRoutingSettings(int bus_wait_time, double bus_velocity);

        std::pair<int, double> GetRoutingSettings() const;
        
		explicit TransportRouter(const TransportCatalogue& tc);
      
        std::optional<VertexId> GetExistsVertexId(const Stop* stop) const;

		std::optional<RouteInfo> GetRouteInfo(VertexId from, VertexId to) const;

	};
}
#include "transport_router.h"  
using namespace transport_catalogue;

        graph::DirectedWeightedGraph<double> TransportRouter::BuildGraph(const TransportCatalogue& tc) {
			std::set<const Stop*> unique_stops;
			for (const auto* bus_ptr : tc.GetAllBuses()) {
				unique_stops.insert(bus_ptr->stops.begin(), bus_ptr->stops.end());
			}

			graph::DirectedWeightedGraph<double> graph(unique_stops.size());
            
			auto [wait_time, velocity] = GetRoutingSettings();
            
			for (const auto* bus_ptr : tc.GetAllBuses()) {
				const std::vector<const Stop*>& stops = bus_ptr->stops;
				for (size_t i = 0, end = stops.size(); i + 1 < end; ++i) {

					const Stop* from = stops[i];

					double edge_weight = wait_time;
					for (size_t j = i + 1; j < stops.size(); ++j) {
						const Stop* span_begin = stops[j - 1];
						const Stop* to = stops[j];
						edge_weight += tc.GetDistance(span_begin, to) * 60. / (velocity * 1000.);
						int stops_count = j - i;
						graph.AddEdge({ bus_ptr->name, GetVertexId(from), GetVertexId(to), edge_weight, stops_count });
					}
				}
			}
			return graph;
		}


        VertexId TransportRouter::GetVertexId(const Stop* stop) {
			static graph::VertexId count = 0;
			if (!stop_to_vertex_id.count(stop)) {
				stop_to_vertex_id[stop] = count;
				id_to_stop[count] = stop;
				++count;
			}
			return stop_to_vertex_id[stop];
		}

        const Stop* TransportRouter::GetStopByVertexID(VertexId id) const {
			return id_to_stop.at(id);
		}

        void TransportRouter::SetRoutingSettings(int bus_wait_time, double bus_velocity) {
           bus_wait_time_ = bus_wait_time;
           bus_velocity_ = bus_velocity;
        }

        std::pair<int, double> TransportRouter::GetRoutingSettings() const {
            return { bus_wait_time_, bus_velocity_ };
        }     
        
		TransportRouter::TransportRouter(const TransportCatalogue& tc) : graph_(BuildGraph(tc)), router_(graph_) {
		}
      
        std::optional<VertexId> TransportRouter::GetExistsVertexId(const Stop* stop) const {
			if (stop_to_vertex_id.count(stop)){
				return stop_to_vertex_id.at(stop);
			}
			else
			{
				return std::nullopt;
			}			
		}


 


		std::optional<TransportRouter::RouteInfo> TransportRouter::GetRouteInfo(VertexId from, VertexId to) const{
			if (auto info = router_.BuildRoute(from, to)) {
				RouteInfo rout_info;
				rout_info.weight = info->weight;
				rout_info.edges.resize(info->edges.size());

				std::transform(info->edges.begin(), info->edges.end(), rout_info.edges.begin(), [&](const graph::EdgeId & edge) {
					auto edge_info = graph_.GetEdge(edge);
					TransportRouter::Edge edge_{ edge_info.bus, GetStopByVertexID(edge_info.from)->name, GetStopByVertexID(edge_info.to)->name, edge_info.weight, edge_info.stops_count };
					return edge_;
					});

				return rout_info;
			}
			else
			{
				return std::nullopt;
			}
		}
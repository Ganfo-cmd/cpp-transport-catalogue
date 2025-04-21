#include "transport_router.h"

#include <iostream>

namespace catalogue
{
    namespace router
    {

        TransportRouter::TransportRouter(RouterSettings &rout_sett, const TransportCatalogue &catalogue, size_t stop_count)
            : wait_time_(rout_sett.wait_time), bus_velocity_(rout_sett.bus_velocity), graph_(stop_count * 2)
        {
            BuildGraph(catalogue);
        }

        void TransportRouter::BuildGraph(const TransportCatalogue &catalogue)
        {
            AddStops(catalogue.GetStopList());
            for (const auto &bus : catalogue.GetBusList())
            {
                size_t from, to;
                bool is_round = bus.is_roundtrip;
                const auto &stops = bus.bus_stops;
                const int stop_count = stops.size();
                for (int i = 0; i < stop_count; ++i)
                {
                    from = vert_id_by_stop_.at(stops[i]);
                    double distance = 0.0, reverse_dist = 0.0;
                    for (int j = i + 1; j < stop_count; ++j)
                    {
                        to = vert_id_by_stop_.at(stops[j]);
                        distance += catalogue.GetDistance(stops[j - 1], stops[j]);
                        graph_.AddEdge({from + 1, to, distance / (bus_velocity_ * (1000.0 / 60.0)), j - i, bus.bus_name});
                        if (!is_round)
                        {
                            reverse_dist += catalogue.GetDistance(stops[j], stops[j -1]);
                            graph_.AddEdge({to + 1, from, reverse_dist / (bus_velocity_ * (1000.0 / 60.0)), j - i, bus.bus_name});
                        }
                    }
                }
            }

            router_ = std::make_unique<graph::Router<double>>(graph_);
        }

        const graph::DirectedWeightedGraph<double> &TransportRouter::GetGraph() const
        {
            return graph_;
        }

        std::optional<TransportRouter::RouteInfo> TransportRouter::GetShortestRoute(const Stop *from, const Stop *to) const
        {
            return router_->BuildRoute(vert_id_by_stop_.at(from), vert_id_by_stop_.at(to));
        }

        void TransportRouter::AddStops(const std::deque<Stop> &stops)
        {
            size_t index = 0;
            for (const auto &stop : stops)
            {
                vert_id_by_stop_[&stop] = index;
                graph_.AddEdge({index, index + 1, wait_time_, 0, stop.stop_name});
                index += 2;
            }
        }
    }
}
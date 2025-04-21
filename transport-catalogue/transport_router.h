#pragma once

#include "router.h"
#include "transport_catalogue.h"

#include <deque>
#include <memory>

namespace catalogue
{
    namespace router
    {
        struct RouterSettings
        {
            double wait_time;
            double bus_velocity;
        };

        class TransportRouter
        {
        public:
            using RouteInfo = graph::Router<double>::RouteInfo;

            TransportRouter(RouterSettings &rout_sett, const TransportCatalogue &catalogue, size_t stop_count);

            void BuildGraph(const TransportCatalogue &catalogue);

            const graph::DirectedWeightedGraph<double> &GetGraph() const;

            std::optional<RouteInfo> GetShortestRoute(const Stop *from, const Stop *to) const;

        private:
            double wait_time_;
            double bus_velocity_;
            std::unordered_map<const Stop *, size_t> vert_id_by_stop_;

            graph::DirectedWeightedGraph<double> graph_;
            std::unique_ptr<graph::Router<double>> router_;

            void AddStops(const std::deque<Stop> &stops);
        };
    }
}

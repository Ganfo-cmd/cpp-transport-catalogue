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
            const double METERS_PER_KILOMETER = 1000.0;
            const double MIN_PER_HOUR = 60.0;
            using RouteInfo = std::vector<graph::Edge<double>>;

            TransportRouter(const RouterSettings &rout_sett, const TransportCatalogue &catalogue);

            std::optional<RouteInfo> GetShortestRoute(const Stop *from, const Stop *to) const;

        private:
            double wait_time_;
            double bus_velocity_;
            std::unordered_map<const Stop *, size_t> vert_id_by_stop_;

            graph::DirectedWeightedGraph<double> graph_;
            std::unique_ptr<graph::Router<double>> router_;

            void BuildGraph(const TransportCatalogue &catalogue);

            void AddStops(const std::deque<Stop> &stops);
        };
    }
}

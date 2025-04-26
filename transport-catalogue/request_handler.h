#pragma once

#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"

namespace catalogue
{
    class RequestHandler
    {
    public:
        using RouteInfo = router::TransportRouter::RouteInfo;

        RequestHandler(const TransportCatalogue &db, const renderer::MapRenderer &renderer, const router::TransportRouter &router);

        svg::Document RenderMap() const;

        const Stop *FindStop(std::string_view name) const;

        BusInfo GetBusInfo(std::string_view name) const;

        const std::unordered_set<const Bus *> &GetStopInfo(std::string_view name) const;

        std::optional<RouteInfo> GetShortestRoute(const Stop *from, const Stop *to) const;

    private:
        const TransportCatalogue &db_;
        const renderer::MapRenderer &renderer_;
        const router::TransportRouter &router_;
    };
}

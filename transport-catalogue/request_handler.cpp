#include "request_handler.h"

#include <algorithm>
#include <map>

namespace catalogue
{
    RequestHandler::RequestHandler(const TransportCatalogue &db, const renderer::MapRenderer &renderer, const router::TransportRouter &router)
        : db_(db), renderer_(renderer), router_(router) {}

    svg::Document RequestHandler::RenderMap() const
    {
        svg::Document result;
        std::vector<geo::Coordinates> stop_coords;
        std::map<std::string_view, geo::Coordinates> stops;
        for (const auto &bus : db_.GetBusList())
        {
            for (const auto &stop : bus.bus_stops)
            {
                const auto &coord = stop->coords;
                stop_coords.push_back({coord.lat, coord.lng});
                stops.insert({stop->stop_name, coord});
            }
        }

        std::vector<std::string_view> buses;
        const std::unordered_map<std::string_view, const Bus *> &busname_to_bus = db_.GetBusNameToBus();
        for (const auto &bus : db_.GetBusList())
        {
            buses.push_back(bus.bus_name);
        }
        std::sort(buses.begin(), buses.end());

        if (!buses.empty())
        {
            result = renderer_.RenderMap(stop_coords, buses, busname_to_bus, stops);
        }

        return result;
    }

    const Stop *RequestHandler::FindStop(std::string_view name) const
    {
        return db_.FindStop(name);
    }

    const std::unordered_set<const Bus *> &RequestHandler::GetStopInfo(std::string_view name) const
    {
        return db_.GetStopInfo(name);
    }

    BusInfo RequestHandler::GetBusInfo(std::string_view name) const
    {
        return db_.GetBusInfo(name);
    }

    std::optional<RequestHandler::RouteInfo> RequestHandler::GetShortestRoute(const Stop *from, const Stop *to) const
    {
        return router_.GetShortestRoute(from, to);
    }
}
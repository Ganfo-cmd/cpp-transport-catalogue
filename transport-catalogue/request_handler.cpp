#include "request_handler.h"

#include <algorithm>
#include <map>

namespace catalogue
{
    RequestHandler::RequestHandler(const TransportCatalogue &db, const renderer::MapRenderer &renderer)
        : db_(db), renderer_(renderer) {}

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

        const renderer::SphereProjector proj = renderer_.GetSphereProjector(stop_coords);

        std::vector<std::string_view> buses;
        const std::unordered_map<std::string_view, const Bus *> &busname_to_bus = db_.GetBusNameToBus();
        for (const auto &bus : db_.GetBusList())
        {
            buses.push_back(bus.bus_name);
        }
        std::sort(buses.begin(), buses.end());

        if (!buses.empty())
        {
            renderer_.RenderBusRoutes(result, proj, buses, busname_to_bus);
            renderer_.RenderRoutesName(result, proj, buses, busname_to_bus);
            renderer_.RenderStopCircle(result, proj, stops);
            renderer_.RenderStopName(result, proj, stops);
        }

        return result;
    }
}
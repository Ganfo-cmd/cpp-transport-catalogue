#include "request_handler.h"

#include <algorithm>

namespace catalogue
{
    RequestHandler::RequestHandler(const TransportCatalogue &db, const renderer::MapRenderer &renderer)
        : db_(db), renderer_(renderer) {}

    const std::vector<geo::Coordinates> RequestHandler::GetStopsCoordinates() const
    {
        std::vector<geo::Coordinates> result;
        for (const auto &bus : db_.GetBusList())
        {
            for (const auto &stop : bus.bus_stops)
            {
                double lat = stop->coords.lat;
                double lng = stop->coords.lng;
                result.push_back({lat, lng});
            }
        }
        return result;
    }

    std::vector<std::pair<std::string_view, geo::Coordinates>> RequestHandler::GetSortedStops() const
    {
        std::vector<std::pair<std::string_view, geo::Coordinates>> result;
        for (const auto &[stop_name, coord] : db_.GetStopList())
        {
            if (db_.GetStopInfo(stop_name).empty())
            {
                continue;
            }
            result.push_back({stop_name, coord});
        }

        std::sort(result.begin(), result.end(), [](const auto &left, const auto &right)
                  { return left.first < right.first; });

        return result;
    }

    svg::Document RequestHandler::RenderMap() const
    {
        svg::Document result;
        std::vector<std::string_view> buses;
        const renderer::SphereProjector proj = renderer_.GetSphereProjector(std::move(GetStopsCoordinates()));

        for (const auto &bus : db_.GetBusList())
        {
            buses.push_back(bus.bus_name);
        }
        std::sort(buses.begin(), buses.end());

        if (!buses.empty())
        {
            RenderBusRoutes(result, proj, buses);
            RenderRoutesName(result, proj, buses);

            std::vector<std::pair<std::string_view, geo::Coordinates>> stops(GetSortedStops());
            RenderStopCircle(result, proj, stops);
            RenderStopName(result, proj, stops);
        }

        return result;
    }

    void RequestHandler::RenderBusRoutes(svg::Document &doc, const renderer::SphereProjector &proj, const std::vector<std::string_view> &buses) const
    {
        std::vector<geo::Coordinates> stops_coordinates;
        int index = 0;
        for (const auto &bus : buses)
        {

            for (const auto &stop : db_.FindBus(bus)->bus_stops)
            {
                double lat = stop->coords.lat;
                double lng = stop->coords.lng;
                stops_coordinates.push_back({lat, lng});
            }

            if (stops_coordinates.empty())
            {
                continue;
            }

            svg::Polyline route;
            for (const auto &coord : stops_coordinates)
            {
                const svg::Point screen_coord = proj(coord);
                route.AddPoint(screen_coord);
            }

            renderer_.SetRouteProperties(route, index);
            ++index;

            doc.Add(route);
            stops_coordinates.clear();
        }
    }

    void RequestHandler::RenderRoutesName(svg::Document &doc, const renderer::SphereProjector &proj, const std::vector<std::string_view> &buses) const
    {
        int index = 0;
        for (const auto &bus : buses)
        {
            const auto &bus_info = db_.FindBus(bus);
            const bool is_roundtrip = bus_info->is_roundtrip;
            const auto &bus_stops = bus_info->bus_stops;
            if (bus_stops.empty())
            {
                continue;
            }

            svg::Text route_label;
            svg::Text route_text;

            const svg::Point screen_coord = proj(bus_stops[0]->coords);
            renderer_.SetRouteNameProperties(route_label, route_text, screen_coord, bus, index);

            doc.Add(route_label);
            doc.Add(route_text);

            if (!is_roundtrip && bus_stops[0] != bus_stops[bus_stops.size() / 2])
            {
                svg::Text second_route_label;
                svg::Text second_route_text;

                const svg::Point sec_screen_coord = proj(bus_stops[bus_stops.size() / 2]->coords);
                renderer_.SetRouteNameProperties(second_route_label, second_route_text, sec_screen_coord, bus, index);

                doc.Add(second_route_label);
                doc.Add(second_route_text);
            }
            ++index;
        }
    }

    void RequestHandler::RenderStopCircle(svg::Document &doc, const renderer::SphereProjector &proj, const std::vector<std::pair<std::string_view, geo::Coordinates>> &stops) const
    {

        for (const auto &[stop_name, coord] : stops)
        {
            svg::Circle stop_icon;
            stop_icon.SetCenter(proj(coord));
            renderer_.SetStopIconProperties(stop_icon);

            doc.Add(stop_icon);
        }
    }

    void RequestHandler::RenderStopName(svg::Document &doc, const renderer::SphereProjector &proj, const std::vector<std::pair<std::string_view, geo::Coordinates>> &stops) const
    {
        for (const auto &[stop_name, coord] : stops)
        {
            svg::Text stop_label;
            svg::Text stop_text;

            const svg::Point screen_coord = proj(coord);
            stop_label.SetPosition(screen_coord);
            stop_text.SetPosition(screen_coord);

            renderer_.SetStopNameProperties(stop_label, stop_text, stop_name);

            doc.Add(stop_label);
            doc.Add(stop_text);
        }
    }
}
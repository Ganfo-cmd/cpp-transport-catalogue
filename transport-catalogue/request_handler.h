#pragma once

#include "map_renderer.h"
#include "transport_catalogue.h"

namespace catalogue
{
    class RequestHandler
    {
    public:
        RequestHandler(const TransportCatalogue &db, const renderer::MapRenderer &renderer);

        // Возвращает информацию о маршруте (запрос Bus)
        // std::optional<BusStat> GetBusStat(const std::string_view &bus_name) const;

        // Возвращает маршруты, проходящие через
        // const std::unordered_set<BusPtr> *GetBusesByStop(const std::string_view &stop_name) const;

        svg::Document RenderMap() const;

        void RenderBusRoutes(svg::Document &doc, const renderer::SphereProjector &proj, const std::vector<std::string_view> &buses) const;

        void RenderRoutesName(svg::Document &doc, const renderer::SphereProjector &proj, const std::vector<std::string_view> &buses) const;

        void RenderStopCircle(svg::Document &doc, const renderer::SphereProjector &proj, const std::vector<std::pair<std::string_view, geo::Coordinates>> &stops) const;

        void RenderStopName(svg::Document &doc, const renderer::SphereProjector &proj, const std::vector<std::pair<std::string_view, geo::Coordinates>> &stops) const;

        const std::vector<geo::Coordinates> GetStopsCoordinates() const;

        std::vector<std::pair<std::string_view, geo::Coordinates>> GetSortedStops() const;

    private:
        const TransportCatalogue &db_;
        const renderer::MapRenderer &renderer_;
    };
}

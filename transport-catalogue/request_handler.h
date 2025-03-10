#pragma once

#include "map_renderer.h"
#include "transport_catalogue.h"

namespace catalogue
{
    class RequestHandler
    {
    public:
        RequestHandler(const TransportCatalogue &db, const renderer::MapRenderer &renderer);

        svg::Document RenderMap() const;

    private:
        const TransportCatalogue &db_;
        const renderer::MapRenderer &renderer_;
    };
}

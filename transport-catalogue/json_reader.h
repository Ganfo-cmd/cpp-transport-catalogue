#pragma once

#include "json.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"

namespace catalogue
{
    namespace json
    {
        struct StatRequests
        {
            int id;
            std::string type;
            std::string name;
            std::string from;
            std::string to;
        };

        void ParseRequests(const Document &doc, TransportCatalogue &catalogue, std::vector<StatRequests> &stat_requests, renderer::RenderSettings &rend_sett, router::RouterSettings &rout_sett);

        void ParseMap(const Dict &dictionary, TransportCatalogue &catalogue, std::vector<StatRequests> &stat_requests, renderer::RenderSettings &rend_sett, router::RouterSettings &rout_sett);

        void ParseBaseRequest(const Node &node, TransportCatalogue &catalogue);

        void ParseStatRequest(const Node &node, std::vector<StatRequests> &stat_requests);

        void ParseRenderSettings(const Node &node, renderer::RenderSettings &rend_sett);

        void ParseRouteSettings(const Node &node, router::RouterSettings &rout_sett);

        svg::Color ParseColor(const Node &node);

        Document GetOutputDocument(RequestHandler &request_handler, std::vector<StatRequests> &stat_requests);
    }
}
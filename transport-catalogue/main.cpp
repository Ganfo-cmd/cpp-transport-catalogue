#include <iostream>
#include <string>
#include <vector>

#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"

using namespace std;
using namespace catalogue;
using namespace catalogue::json;
using namespace catalogue::renderer;
using namespace catalogue::router;

int main()
{
    Document doc;
    TransportCatalogue catalogue;
    RenderSettings rend_sett;
    RouterSettings rout_sett;
    std::vector<StatRequests> stat_requests;

    doc = Load(std::cin);
    ParseRequests(doc, catalogue, stat_requests, rend_sett, rout_sett);

    MapRenderer map_rend(rend_sett);
    TransportRouter router(rout_sett, catalogue);
    RequestHandler request_handler(catalogue, map_rend, router);
    Document output = GetOutputDocument(request_handler, stat_requests);
    Print(output, std::cout);
}
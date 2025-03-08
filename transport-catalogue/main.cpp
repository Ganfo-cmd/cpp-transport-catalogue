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

int main()
{
    Document doc;
    TransportCatalogue catalogue;
    RenderSettings rend_sett;
    std::vector<StatRequests> stat_requests;

    doc = Load(std::cin);
    ParseRequests(doc, catalogue, stat_requests, rend_sett);

    Document output = GetOutputDocument(catalogue, stat_requests, rend_sett);
    Print(output, std::cout);
}
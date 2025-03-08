#include "json_reader.h"

#include <set>
#include <sstream>

namespace catalogue
{
    namespace json
    {
        using namespace std::literals;

        void ParseRequests(const Document &doc, TransportCatalogue &catalogue, std::vector<StatRequests> &stat_requests, renderer::RenderSettings &rend_sett)
        {
            const auto &root = doc.GetRoot();
            if (root.IsMap())
            {
                ParseMap(root.AsMap(), catalogue, stat_requests, rend_sett);
            }
            else
            {
                std::cerr << "Error: incorrect input data format"sv;
            }
        }

        void ParseMap(const Dict &dictionary, TransportCatalogue &catalogue, [[maybe_unused]] std::vector<StatRequests> &stat_requests, renderer::RenderSettings &rend_sett)
        {
            const auto it_base_req = dictionary.find("base_requests");
            const auto end = dictionary.end();
            if (it_base_req != end)
            {
                ParseBaseRequest(it_base_req->second, catalogue);
            }
            else
            {
                std::cerr << "Error: base_requests is missing in the request"sv;
                return;
            }

            const auto it_stat_req = dictionary.find("stat_requests");
            if (it_stat_req != end)
            {
                ParseStatRequest(it_stat_req->second, stat_requests);
            }
            else
            {
                std::cerr << "Error: stat_requests is missing in the request"sv;
                return;
            }

            const auto it_render_sett = dictionary.find("render_settings");
            if (it_render_sett != end)
            {
                ParseRenderSettings(it_render_sett->second, rend_sett);
            }
            else
            {
                std::cerr << "Error: render_settings is missing in the request"sv;
                return;
            }
        }

        void ParseRenderSettings(const Node &node, renderer::RenderSettings &rend_sett)
        {
            if (!node.IsMap())
            {
                std::cerr << "Error: content of stat_requests is not a array"sv;
            }

            const auto &dictionary = node.AsMap();
            rend_sett.width = dictionary.at("width").AsDouble();
            rend_sett.height = dictionary.at("height").AsDouble();
            rend_sett.padding = dictionary.at("padding").AsDouble();

            rend_sett.line_width = dictionary.at("line_width").AsDouble();
            rend_sett.stop_radius = dictionary.at("stop_radius").AsDouble();

            rend_sett.bus_label_font_size = dictionary.at("bus_label_font_size").AsInt();
            const auto &bus_label_offset = dictionary.at("bus_label_offset").AsArray();
            rend_sett.bus_label_offset = std::make_pair(bus_label_offset[0].AsDouble(), bus_label_offset[1].AsDouble());

            rend_sett.stop_label_font_size = dictionary.at("stop_label_font_size").AsInt();
            const auto &stop_label_offset = dictionary.at("stop_label_offset").AsArray();
            rend_sett.stop_label_offset = std::make_pair(stop_label_offset[0].AsDouble(), stop_label_offset[1].AsDouble());

            rend_sett.underlayer_width = dictionary.at("underlayer_width").AsDouble();
            const auto &und_color = dictionary.at("underlayer_color");
            if (und_color.IsString())
            {
                rend_sett.underlayer_color = und_color.AsString();
            }
            else if (und_color.IsArray())
            {
                const auto &und_color_arr = und_color.AsArray();
                uint8_t red = und_color_arr[0].AsInt();
                uint8_t green = und_color_arr[1].AsInt();
                uint8_t blue = und_color_arr[2].AsInt();

                if (und_color_arr.size() == 3)
                {

                    rend_sett.underlayer_color = std::move(svg::Color{svg::Rgb{red, green, blue}});
                }
                else
                {
                    double opacity = und_color_arr[3].AsDouble();
                    rend_sett.underlayer_color = std::move(svg::Color{svg::Rgba{red, green, blue, opacity}});
                }
            }

            std::vector<svg::Color> &color_pal = rend_sett.color_palette;
            for (const auto &color : dictionary.at("color_palette").AsArray())
            {
                if (color.IsString())
                {
                    color_pal.push_back(color.AsString());
                }
                else if (color.IsArray())
                {
                    const auto &color_arr = color.AsArray();
                    uint8_t red = color_arr[0].AsInt();
                    uint8_t green = color_arr[1].AsInt();
                    uint8_t blue = color_arr[2].AsInt();
                    if (color_arr.size() == 3)
                    {
                        color_pal.push_back(svg::Rgb{red, green, blue});
                    }
                    else
                    {
                        double opacity = color_arr[3].AsDouble();
                        color_pal.push_back(svg::Rgba{red, green, blue, opacity});
                    }
                }
            }
        }

        StatRequests ParseCommandDescription(const Node &node)
        {
            const auto &dict = node.AsMap();
            const auto &type = dict.at("type").AsString();
            if (type == "Map")
            {
                return {dict.at("id").AsInt(), type, ""};
            }

            return {dict.at("id").AsInt(), type, dict.at("name").AsString()};
        }

        void ParseStatRequest(const Node &node, std::vector<StatRequests> &stat_requests)
        {
            if (!node.IsArray())
            {
                std::cerr << "Error: content of stat_requests is not a array"sv;
            }

            for (const auto &req : node.AsArray())
            {
                stat_requests.push_back(std::move(ParseCommandDescription(req)));
            }
        }

        std::vector<std::string_view> ParseRoute(const Array &array, bool is_roundtrip)
        {
            std::vector<std::string_view> route;
            for (const auto &node : array)
            {
                route.push_back(node.AsString());
            }

            if (!is_roundtrip)
            {
                route.insert(route.end(), std::next(route.rbegin()), route.rend());
            }

            return route;
        }

        void ParseBusRequest(const Dict &dict, TransportCatalogue &catalogue)
        {
            const bool is_roundtrip = dict.at("is_roundtrip").AsBool();
            std::vector<std::string_view> stops = ParseRoute(dict.at("stops").AsArray(), is_roundtrip);

            catalogue.AddBus(dict.at("name").AsString(), stops, is_roundtrip);
        }

        void ParseStopRequest(const Dict &dict, TransportCatalogue &catalogue)
        {
            double lat = dict.at("latitude").AsDouble();
            double lon = dict.at("longitude").AsDouble();
            catalogue.AddStop(dict.at("name").AsString(), geo::Coordinates{lat, lon});
        }

        void ParseStopDistance(const Dict &dict, TransportCatalogue &catalogue)
        {
            for (const auto &[other_stop, node] : dict.at("road_distances").AsMap())
            {
                catalogue.SetDistance(dict.at("name").AsString(), other_stop, node.AsInt());
            }
        }

        void ParseBaseRequest(const Node &node, TransportCatalogue &catalogue)
        {
            if (!node.IsArray())
            {
                std::cerr << "Error: content of base_requests is not a array"sv;
            }

            const auto &array = node.AsArray();
            for (const auto &req : array)
            {
                const Dict &dict = req.AsMap();
                std::string_view req_type = dict.at("type").AsString();

                if (req_type == "Stop"sv)
                {
                    ParseStopRequest(dict, catalogue);
                }
            }

            for (const auto &req : array)
            {
                const Dict &dict = req.AsMap();
                std::string_view req_type = dict.at("type").AsString();

                if (req_type == "Stop"sv)
                {
                    ParseStopDistance(dict, catalogue);
                }
            }

            for (const auto &req : array)
            {
                const Dict &dict = req.AsMap();
                std::string_view req_type = dict.at("type").AsString();

                if (req_type == "Bus"sv)
                {
                    ParseBusRequest(dict, catalogue);
                }
            }
        }

        struct BusPtrComparator
        {
            bool operator()(const Bus *left, const Bus *right) const
            {
                return left->bus_name < right->bus_name;
            }
        };

        Dict GetStopInfo(TransportCatalogue &catalogue, int req_id, std::string_view name)
        {
            if (catalogue.FindStop(name) == nullptr)
            {
                return Dict{{"error_message", "not found"s},
                            {"request_id", req_id}};
            }

            Array result;
            const auto &buses = catalogue.GetStopInfo(name);

            const std::set<const Bus *, BusPtrComparator> sorted_buses(buses.begin(), buses.end());
            for (const auto &bus : sorted_buses)
            {
                result.push_back(bus->bus_name);
            }

            return Dict{{"buses", result}, {"request_id", req_id}};
        }

        Dict GetBusInfo(TransportCatalogue &catalogue, int req_id, std::string_view name)
        {
            const auto &bus_info = catalogue.GetBusInfo(name);
            if (bus_info.stops_count == 0)
            {
                return Dict{{"error_message", "not found"s},
                            {"request_id", req_id}};
            }

            return Dict{{"curvature", bus_info.curvature},
                        {"request_id", req_id},
                        {"route_length", int(bus_info.route_length)},
                        {"stop_count", bus_info.stops_count},
                        {"unique_stop_count", bus_info.unique_stops}};
        }

        Dict GetMap(TransportCatalogue &catalogue, int id, renderer::RenderSettings &rend_sett)
        {
            std::ostringstream out;
            renderer::MapRenderer map_rend(rend_sett);
            RequestHandler req_handler(catalogue, map_rend);
            svg::Document output = req_handler.RenderMap();
            output.Render(out);

            return Dict{{"map", Node(out.str())},
                        {"request_id", id}};
        }

        Document GetOutputDocument(TransportCatalogue &catalogue, std::vector<StatRequests> &stat_requests, renderer::RenderSettings &rend_sett)
        {
            Array result;

            for (const auto &[id, type, name] : stat_requests)
            {
                if (type == "Stop")
                {
                    result.push_back(GetStopInfo(catalogue, id, name));
                }

                if (type == "Bus")
                {
                    result.push_back(GetBusInfo(catalogue, id, name));
                }

                if (type == "Map")
                {
                    result.push_back(GetMap(catalogue, id, rend_sett));
                }
            }

            return Document{result};
        }

    } // namespace json
} // namespace catalogue
#include "json_reader.h"
#include "json_builder.h"

#include <set>
#include <sstream>

namespace catalogue
{
    namespace json
    {
        using namespace std::literals;

        void ParseRequests(const Document &doc, TransportCatalogue &catalogue, std::vector<StatRequests> &stat_requests, renderer::RenderSettings &rend_sett, router::RouterSettings &rout_sett)
        {
            const auto &root = doc.GetRoot();
            if (root.IsMap())
            {
                ParseMap(root.AsMap(), catalogue, stat_requests, rend_sett, rout_sett);
            }
            else
            {
                std::cerr << "Error: incorrect input data format"sv;
            }
        }

        void ParseMap(const Dict &dictionary, TransportCatalogue &catalogue, [[maybe_unused]] std::vector<StatRequests> &stat_requests, renderer::RenderSettings &rend_sett, router::RouterSettings &rout_sett)
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

            const auto it_rout_sett = dictionary.find("routing_settings");
            if (it_rout_sett != end)
            {
                ParseRouteSettings(it_rout_sett->second, rout_sett);
            }
            else
            {
                std::cerr << "Error: routing_settings is missing in the request"sv;
                return;
            }
        }

        svg::Color ParseColor(const Node &node)
        {
            if (node.IsString())
            {
                return node.AsString();
            }
            else if (node.IsArray())
            {
                const auto &und_color_arr = node.AsArray();
                uint8_t red = und_color_arr[0].AsInt();
                uint8_t green = und_color_arr[1].AsInt();
                uint8_t blue = und_color_arr[2].AsInt();

                if (und_color_arr.size() == 3)
                {

                    return svg::Color{svg::Rgb{red, green, blue}};
                }
                else
                {
                    double opacity = und_color_arr[3].AsDouble();
                    return svg::Color{svg::Rgba{red, green, blue, opacity}};
                }
            }
            return svg::Color();
        }

        void ParseRenderSettings(const Node &node, renderer::RenderSettings &rend_sett)
        {
            if (!node.IsMap())
            {
                std::cerr << "Error: content of render_settings is not a dict"sv;
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
            rend_sett.underlayer_color = std::move(ParseColor(dictionary.at("underlayer_color")));

            std::vector<svg::Color> &color_pal = rend_sett.color_palette;
            for (const auto &color : dictionary.at("color_palette").AsArray())
            {
                color_pal.push_back(std::move(ParseColor(color)));
            }
        }

        void ParseRouteSettings(const Node &node, router::RouterSettings &rout_sett)
        {
            if (!node.IsMap())
            {
                std::cerr << "Error: content of routing_settings is not a dict"sv;
            }

            const auto &dictionary = node.AsMap();
            rout_sett.bus_velocity = dictionary.at("bus_velocity").AsDouble();
            rout_sett.wait_time = dictionary.at("bus_wait_time").AsDouble();
        }

        StatRequests ParseCommandDescription(const Node &node)
        {
            const auto &dict = node.AsMap();
            const auto &type = dict.at("type").AsString();
            if (type == "Map")
            {
                return {dict.at("id").AsInt(), type, "", "", ""};
            }

            if (type == "Route")
            {
                return {dict.at("id").AsInt(), type, "", dict.at("from").AsString(), dict.at("to").AsString()};
            }

            return {dict.at("id").AsInt(), type, dict.at("name").AsString(), "", ""};
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

        void GetStopInfo(RequestHandler &request_handler, std::string_view name, json::Builder &json_builder)
        {
            if (request_handler.FindStop(name) == nullptr)
            {
                json_builder.Key("error_message").Value("not found");
                return;
            }

            json_builder.Key("buses").StartArray();

            const auto &buses = request_handler.GetStopInfo(name);
            const std::set<const Bus *, BusPtrComparator> sorted_buses(buses.begin(), buses.end());
            for (const auto &bus : sorted_buses)
            {
                json_builder.Value(bus->bus_name);
            }

            json_builder.EndArray();
        }

        void GetBusInfo(RequestHandler &request_handler, std::string_view name, json::Builder &json_builder)
        {
            const auto &bus_info = request_handler.GetBusInfo(name);
            if (bus_info.stops_count == 0)
            {
                json_builder.Key("error_message").Value("not found");
                return;
            }

            json_builder
                .Key("curvature")
                .Value(bus_info.curvature)
                .Key("route_length")
                .Value(int(bus_info.route_length))
                .Key("stop_count")
                .Value(bus_info.stops_count)
                .Key("unique_stop_count")
                .Value(bus_info.unique_stops);
        }

        void GetMap(RequestHandler &request_handler, json::Builder &json_builder)
        {
            std::ostringstream out;
            svg::Document output = request_handler.RenderMap();
            output.Render(out);

            json_builder.Key("map").Value(out.str());
        }

        void GetRouteInfo(RequestHandler &request_handler, json::Builder &json_builder, std::string_view from, std::string_view to)
        {
            const Stop *stop_from = request_handler.FindStop(from);
            const Stop *stop_to = request_handler.FindStop(to);
            const auto &route_info = request_handler.GetShortestRoute(stop_from, stop_to);

            if (!route_info)
            {
                json_builder.Key("error_message").Value("not found");
                return;
            }

            json_builder.Key("items").StartArray();
            double total_time = 0.0;
            for (const auto &edge : route_info.value())
            {
                json_builder.StartDict();
                if (edge.span_count == 0)
                {
                    json_builder
                        .Key("type")
                        .Value("Wait")
                        .Key("stop_name")
                        .Value(std::string(edge.name))
                        .Key("time")
                        .Value(edge.weight);
                }
                else
                {
                    json_builder
                        .Key("type")
                        .Value("Bus")
                        .Key("bus")
                        .Value(std::string(edge.name))
                        .Key("span_count")
                        .Value(edge.span_count)
                        .Key("time")
                        .Value(edge.weight);
                }
                json_builder.EndDict();
                total_time += edge.weight;
            }

            json_builder.EndArray();
            json_builder.Key("total_time").Value(total_time);
        }

        Document GetOutputDocument(RequestHandler &request_handler, std::vector<StatRequests> &stat_requests)
        {
            json::Builder json_builder;
            json_builder.StartArray();

            for (const auto &[id, type, name, from, to] : stat_requests)
            {
                json_builder.StartDict().Key("request_id").Value(id);
                if (type == "Stop")
                {
                    GetStopInfo(request_handler, name, json_builder);
                }

                if (type == "Bus")
                {
                    GetBusInfo(request_handler, name, json_builder);
                }

                if (type == "Map")
                {
                    GetMap(request_handler, json_builder);
                }

                if (type == "Route")
                {
                    GetRouteInfo(request_handler, json_builder, from, to);
                }

                json_builder.EndDict();
            }

            json_builder.EndArray();
            return Document{json_builder.Build()};
        }

    } // namespace json
} // namespace catalogue
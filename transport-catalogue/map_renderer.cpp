#include "map_renderer.h"

namespace catalogue
{
    namespace renderer
    {
        svg::Point SphereProjector::operator()(geo::Coordinates coords) const
        {
            return {
                (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_};
        }

        MapRenderer::MapRenderer(RenderSettings &render_settings)
            : render_settings_(render_settings) {}

        void MapRenderer::SetRouteProperties(svg::Polyline &route, int index) const
        {
            const auto &color_pal = render_settings_.color_palette;
            if (color_pal.size() > 0)
            {
                route.SetFillColor("none");
                route.SetStrokeColor(color_pal[index % color_pal.size()]);
                route.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
                route.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
                route.SetStrokeWidth(render_settings_.line_width);
            }
        }

        void MapRenderer::SetRouteNameProperties(svg::Text &route_label, svg::Text &route_text, const svg::Point &screen_coord, std::string_view bus_name, int index) const
        {
            const auto &color_pal = render_settings_.color_palette;
            if (color_pal.size() > 0)
            {
                route_label.SetFillColor(render_settings_.underlayer_color);
                route_text.SetFillColor(color_pal[index % color_pal.size()]);

                route_label.SetStrokeColor(render_settings_.underlayer_color);
                route_label.SetStrokeWidth(render_settings_.underlayer_width);
                route_label.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
                route_label.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

                route_label.SetPosition(screen_coord);
                route_text.SetPosition(screen_coord);

                const svg::Point offset{render_settings_.bus_label_offset.first, render_settings_.bus_label_offset.second};
                route_label.SetOffset(offset);
                route_text.SetOffset(offset);

                route_label.SetFontSize(render_settings_.bus_label_font_size);
                route_text.SetFontSize(render_settings_.bus_label_font_size);

                route_label.SetFontFamily("Verdana");
                route_text.SetFontFamily("Verdana");

                route_label.SetFontWeight("bold");
                route_text.SetFontWeight("bold");

                route_label.SetData(std::string(bus_name));
                route_text.SetData(std::string(bus_name));
            }
        }

        void MapRenderer::SetStopIconProperties(svg::Circle &stop_icon) const
        {
            stop_icon.SetRadius(render_settings_.stop_radius);
            stop_icon.SetFillColor("white");
        }

        void MapRenderer::SetStopNameProperties(svg::Text &stop_label, svg::Text &stop_text, std::string_view stop_name) const
        {
            const svg::Point offset{render_settings_.stop_label_offset.first, render_settings_.stop_label_offset.second};
            stop_label.SetOffset(offset);
            stop_text.SetOffset(offset);

            stop_label.SetFontSize(render_settings_.stop_label_font_size);
            stop_text.SetFontSize(render_settings_.stop_label_font_size);

            stop_label.SetFontFamily("Verdana");
            stop_text.SetFontFamily("Verdana");

            stop_label.SetData(std::string(stop_name));
            stop_text.SetData(std::string(stop_name));

            stop_label.SetFillColor(render_settings_.underlayer_color);
            stop_text.SetFillColor("black");

            stop_label.SetStrokeColor(render_settings_.underlayer_color);
            stop_label.SetStrokeWidth(render_settings_.underlayer_width);
            stop_label.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            stop_label.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        }

        void MapRenderer::RenderBusRoutes(svg::Document &doc, const SphereProjector &proj, const std::vector<std::string_view> &buses, const std::unordered_map<std::string_view, const Bus *> &busname_to_bus) const
        {
            std::vector<geo::Coordinates> stops_coordinates;
            int index = 0;
            for (const auto &bus : buses)
            {

                for (const auto &stop : busname_to_bus.at(bus)->bus_stops)
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

                SetRouteProperties(route, index);
                ++index;

                doc.Add(route);
                stops_coordinates.clear();
            }
        }

        void MapRenderer::RenderRoutesName(svg::Document &doc, const SphereProjector &proj, const std::vector<std::string_view> &buses, const std::unordered_map<std::string_view, const Bus *> &busname_to_bus) const
        {
            int index = 0;
            for (const auto &bus : buses)
            {
                const auto &bus_info = busname_to_bus.at(bus);
                const bool is_roundtrip = bus_info->is_roundtrip;
                const auto &bus_stops = bus_info->bus_stops;
                if (bus_stops.empty())
                {
                    continue;
                }

                svg::Text route_label;
                svg::Text route_text;

                const svg::Point screen_coord = proj(bus_stops[0]->coords);
                SetRouteNameProperties(route_label, route_text, screen_coord, bus, index);

                doc.Add(route_label);
                doc.Add(route_text);

                if (!is_roundtrip && bus_stops[0] != bus_stops[bus_stops.size() / 2])
                {
                    svg::Text second_route_label;
                    svg::Text second_route_text;

                    const svg::Point sec_screen_coord = proj(bus_stops[bus_stops.size() / 2]->coords);
                    SetRouteNameProperties(second_route_label, second_route_text, sec_screen_coord, bus, index);

                    doc.Add(second_route_label);
                    doc.Add(second_route_text);
                }
                ++index;
            }
        }

        void MapRenderer::RenderStopCircle(svg::Document &doc, const SphereProjector &proj, const std::map<std::string_view, geo::Coordinates> &stops) const
        {

            for (const auto &[stop_name, coord] : stops)
            {
                svg::Circle stop_icon;
                stop_icon.SetCenter(proj(coord));
                SetStopIconProperties(stop_icon);

                doc.Add(stop_icon);
            }
        }

        void MapRenderer::RenderStopName(svg::Document &doc, const SphereProjector &proj, const std::map<std::string_view, geo::Coordinates> &stops) const
        {
            for (const auto &[stop_name, coord] : stops)
            {
                svg::Text stop_label;
                svg::Text stop_text;

                const svg::Point screen_coord = proj(coord);
                stop_label.SetPosition(screen_coord);
                stop_text.SetPosition(screen_coord);

                SetStopNameProperties(stop_label, stop_text, stop_name);

                doc.Add(stop_label);
                doc.Add(stop_text);
            }
        }

        svg::Document MapRenderer::RenderMap(const std::vector<geo::Coordinates> &stop_coords, const std::vector<std::string_view> &buses, const std::unordered_map<std::string_view, const Bus *> &busname_to_bus, const std::map<std::string_view, geo::Coordinates> &stops) const
        {
            svg::Document result;
            const auto &proj = GetSphereProjector(stop_coords);
            RenderBusRoutes(result, proj, buses, busname_to_bus);
            RenderRoutesName(result, proj, buses, busname_to_bus);
            RenderStopCircle(result, proj, stops);
            RenderStopName(result, proj, stops);

            return result;
        }

        SphereProjector MapRenderer::GetSphereProjector(const std::vector<geo::Coordinates> &stops_coordinates) const
        {
            return SphereProjector{stops_coordinates.begin(), stops_coordinates.end(),
                                   render_settings_.width, render_settings_.height, render_settings_.padding};
        }
    } // namespace renderer
} // namespace catalogue
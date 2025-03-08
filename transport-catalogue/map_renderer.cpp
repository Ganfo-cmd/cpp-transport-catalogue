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

        SphereProjector MapRenderer::GetSphereProjector(const std::vector<geo::Coordinates> &stops_coordinates) const
        {
            return SphereProjector{stops_coordinates.begin(), stops_coordinates.end(),
                                   render_settings_.width, render_settings_.height, render_settings_.padding};
        }
    } // namespace renderer
} // namespace catalogue
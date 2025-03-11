#pragma once

#include "domain.h"
#include "geo.h"
#include "svg.h"

#include <algorithm>
#include <map>
#include <unordered_map>

namespace catalogue
{
    namespace renderer
    {
        inline const double EPSILON = 1e-6;

        class SphereProjector
        {
        public:
            // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
            template <typename PointInputIt>
            SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                            double max_width, double max_height, double padding);

            // Проецирует широту и долготу в координаты внутри SVG-изображения
            svg::Point operator()(geo::Coordinates coords) const;

        private:
            bool IsZero(double value)
            {
                return std::abs(value) < EPSILON;
            }

            double padding_;
            double min_lon_ = 0;
            double max_lat_ = 0;
            double zoom_coeff_ = 0;
        };

        struct RenderSettings
        {
            double width;
            double height;

            double padding;

            double line_width;
            double stop_radius;

            int bus_label_font_size;
            std::pair<double, double> bus_label_offset;

            int stop_label_font_size;
            std::pair<double, double> stop_label_offset;

            svg::Color underlayer_color;
            double underlayer_width;

            std::vector<svg::Color> color_palette;
        };

        class MapRenderer
        {
        public:
            MapRenderer(RenderSettings &render_settings);

            void SetRouteProperties(svg::Polyline &route, int index) const;

            void SetRouteNameProperties(svg::Text &route_label, svg::Text &route_text, const svg::Point &screen_coord, std::string_view bus_name, int index) const;

            void SetStopIconProperties(svg::Circle &stop_icon) const;

            void SetStopNameProperties(svg::Text &stop_label, svg::Text &stop_text, std::string_view stop_name) const;

            void RenderBusRoutes(svg::Document &doc, const SphereProjector &proj, const std::vector<std::string_view> &buses, const std::unordered_map<std::string_view, const Bus *> &busname_to_bus) const;

            void RenderRoutesName(svg::Document &doc, const SphereProjector &proj, const std::vector<std::string_view> &buses, const std::unordered_map<std::string_view, const Bus *> &busname_to_bus) const;

            void RenderStopCircle(svg::Document &doc, const SphereProjector &proj, const std::map<std::string_view, geo::Coordinates> &stops) const;

            void RenderStopName(svg::Document &doc, const SphereProjector &proj, const std::map<std::string_view, geo::Coordinates> &stops) const;

            svg::Document RenderMap(const std::vector<geo::Coordinates> &stop_coords, const std::vector<std::string_view> &buses, const std::unordered_map<std::string_view, const Bus *> &busname_to_bus, const std::map<std::string_view, geo::Coordinates> &stops) const;

            SphereProjector GetSphereProjector(const std::vector<geo::Coordinates> &stops_coordinates) const;

        private:
            RenderSettings &render_settings_;
        };

        template <typename PointInputIt>
        SphereProjector::SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                                         double max_width, double max_height, double padding)
            : padding_(padding)
        {
            if (points_begin == points_end)
            {
                return;
            }

            // Находим точки с минимальной и максимальной долготой
            const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs)
                { return lhs.lng < rhs.lng; });
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;

            // Находим точки с минимальной и максимальной широтой
            const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs)
                { return lhs.lat < rhs.lat; });
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            // Вычисляем коэффициент масштабирования вдоль координаты x
            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_))
            {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            // Вычисляем коэффициент масштабирования вдоль координаты y
            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat))
            {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom)
            {
                // Коэффициенты масштабирования по ширине и высоте ненулевые,
                // берём минимальный из них
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            }
            else if (width_zoom)
            {
                // Коэффициент масштабирования по ширине ненулевой, используем его
                zoom_coeff_ = *width_zoom;
            }
            else if (height_zoom)
            {
                // Коэффициент масштабирования по высоте ненулевой, используем его
                zoom_coeff_ = *height_zoom;
            }
        }
    } // namespace renderer
} // namespace catalogue
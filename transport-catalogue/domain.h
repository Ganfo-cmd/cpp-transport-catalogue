#pragma once

#include "geo.h"
#include <vector>
#include <string>
/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки.
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */

struct Stop
{
    std::string stop_name;
    geo::Coordinates coords;
};

struct Bus
{
    std::string bus_name;
    std::vector<const Stop *> bus_stops;
    bool is_roundtrip;
};

struct BusInfo
{
    int stops_count = 0;
    int unique_stops = 0;
    double route_length = 0.0;
    double curvature = 0.0;
};
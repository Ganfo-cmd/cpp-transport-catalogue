#include "transport_catalogue.h"

#include <unordered_set>

namespace catalogue
{

    void TransportCatalogue::AddStop(const std::string &name, Coordinates coords)
    {
        stops_.push_back({name, coords});
        stopname_to_stop_.insert({stops_.back().stop_name, &stops_.back()});
        bus_by_name_[stops_.back().stop_name];
    }

    const Stop *TransportCatalogue::FindStop(std::string_view name) const
    {
        const auto it = stopname_to_stop_.find(name);
        if (it == stopname_to_stop_.end())
        {
            return nullptr;
        }
        return it->second;
    }

    void TransportCatalogue::AddBus(const std::string &name, const std::vector<std::string_view> &stops)
    {
        std::vector<const Stop *> bus_stops;
        for (const auto &stop : stops)
        {
            bus_stops.push_back(stopname_to_stop_.at(stop));
        }

        buses_.push_back({name, move(bus_stops)});
        busname_to_bus_.insert({buses_.back().bus_name, &buses_.back()});

        for (const auto &stop : stops)
        {
            bus_by_name_.at(stop).insert(&buses_.back());
        }
    }

    const Bus *TransportCatalogue::FindBus(const std::string_view name) const
    {
        const auto it = busname_to_bus_.find(name);
        if (it == busname_to_bus_.end())
        {
            return nullptr;
        }
        return it->second;
    }

    BusInfo TransportCatalogue::GetBusInfo(std::string_view name) const
    {
        BusInfo bus_info;
        if (FindBus(name) == nullptr)
        {
            return bus_info;
        }

        const auto &bus_stops = busname_to_bus_.at(name)->bus_stops;
        bus_info.stops_count = bus_stops.size();

        std::unordered_set<std::string_view> unique_stop;
        for (const auto &stop : bus_stops)
        {
            unique_stop.insert(stop->stop_name);
        }
        bus_info.unique_stops = unique_stop.size();

        double loc_route_length = 0;
        for (auto iter = bus_stops.begin(); iter != std::prev(bus_stops.end());)
        {
            const auto other_iter = iter;
            ++iter;
            loc_route_length += ComputeDistance((*other_iter)->coords, (*iter)->coords);
        }
        bus_info.route_length = loc_route_length;

        return bus_info;
    }

    const std::unordered_set<const Bus *> *TransportCatalogue::GetStopInfo(std::string_view name) const
    {
        if (FindStop(name) != nullptr)
        {
            return &(bus_by_name_.at(name));
        }

        return nullptr;
    }
}

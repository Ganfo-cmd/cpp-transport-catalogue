#include "transport_catalogue.h"

#include <unordered_set>

namespace catalogue
{

    void TransportCatalogue::AddStop(const std::string &name, Coordinates coords)
    {
        stops_.push_back({name, coords});
        stopname_to_stop_.insert({stops_.back().stop_name, &stops_.back()});
        stop_to_buses_[stops_.back().stop_name];
    }

    const Stop *TransportCatalogue::FindStop(std::string_view name) const
    {
        if (stopname_to_stop_.find(name) == stopname_to_stop_.end())
        {
            return nullptr;
        }
        return stopname_to_stop_.at(name);
    }

    void TransportCatalogue::AddBus(const std::string &name, std::vector<std::string_view> stops)
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
            stop_to_buses_.at(stop).insert(&buses_.back());
        }
    }

    const Bus *TransportCatalogue::FindBus(const std::string_view name) const
    {
        if (busname_to_bus_.find(name) == busname_to_bus_.end())
        {
            return nullptr;
        }
        return busname_to_bus_.at(name);
    }

    BusInfo TransportCatalogue::GetBusInfo(std::string_view name) const
    {
        BusInfo bus_info;
        if (FindBus(name) != nullptr)
        {
            bus_info.stops_count = busname_to_bus_.at(name)->bus_stops.size();

            std::unordered_set<std::string_view> unique_stop;
            for (const auto &stop : busname_to_bus_.at(name)->bus_stops)
            {
                unique_stop.insert(stop->stop_name);
            }
            bus_info.unique_stops = unique_stop.size();

            double loc_route_length = 0;
            for (auto iter = busname_to_bus_.at(name)->bus_stops.begin(); iter != std::prev(busname_to_bus_.at(name)->bus_stops.end());)
            {
                const auto other_iter = iter;
                ++iter;
                loc_route_length += ComputeDistance((*other_iter)->coords, (*iter)->coords);
            }
            bus_info.route_length = loc_route_length;

            return bus_info;
        }

        return bus_info;
    }

    const std::set<const Bus *, detail::BusPtrComparator> *TransportCatalogue::GetStopInfo(std::string_view name) const
    {
        if (FindStop(name) != nullptr)
        {
            return &(stop_to_buses_.at(name));
        }

        return nullptr;
    }
}

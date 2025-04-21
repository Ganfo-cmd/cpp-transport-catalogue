#include "transport_catalogue.h"

#include <unordered_set>

namespace catalogue
{
    void TransportCatalogue::AddStop(const std::string &name, geo::Coordinates coords)
    {
        stops_.push_back({name, coords});
        stopname_to_stop_.insert({stops_.back().stop_name, &stops_.back()});
        buses_by_stopname_[stops_.back().stop_name];
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

    void TransportCatalogue::AddBus(const std::string &name, const std::vector<std::string_view> &stops, bool is_roundtrip)
    {
        std::vector<const Stop *> bus_stops;
        for (const auto &stop : stops)
        {
            bus_stops.push_back(stopname_to_stop_.at(stop));
        }

        buses_.push_back({name, move(bus_stops), is_roundtrip});
        busname_to_bus_.insert({buses_.back().bus_name, &buses_.back()});

        for (const auto &stop : stops)
        {
            buses_by_stopname_.at(stop).insert(&buses_.back());
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

    void TransportCatalogue::SetDistance(std::string_view stop_name, std::string_view other_stop, int distance)
    {
        const auto from = FindStop(stop_name);
        const auto to = FindStop(other_stop);
        distances_by_stops_[{from, to}] = distance;
    }

    int TransportCatalogue::GetDistance(const Stop *stop, const Stop *other_stop) const
    {
        if (distances_by_stops_.count({stop, other_stop}))
        {
            return distances_by_stops_.at({stop, other_stop});
        }
        else if (distances_by_stops_.count({other_stop, stop}))
        {
            return distances_by_stops_.at({other_stop, stop});
        }
        else
        {
            return 0;
        }
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

        double loc_route_geo_length = 0.0;
        double loc_route_coord_length = 0.0;
        for (auto iter = bus_stops.begin(); iter != std::prev(bus_stops.end());)
        {
            const auto other_iter = iter;
            ++iter;

            loc_route_coord_length += ComputeDistance((*other_iter)->coords, (*iter)->coords);

            loc_route_geo_length += GetDistance(*other_iter, *iter);
        }

        bus_info.route_length = loc_route_geo_length;
        bus_info.curvature = loc_route_geo_length / loc_route_coord_length;

        return bus_info;
    }

    const std::unordered_set<const Bus *> &TransportCatalogue::GetStopInfo(std::string_view name) const
    {
        static const std::unordered_set<const Bus *> empty;

        const auto it = buses_by_stopname_.find(name);
        if (it != buses_by_stopname_.end())
        {
            return it->second;
        }

        return empty;
    }

    const std::unordered_map<std::string_view, const Bus *> &TransportCatalogue::GetBusNameToBus() const
    {
        return busname_to_bus_;
    }

    const std::deque<Bus> &TransportCatalogue::GetBusList() const
    {
        return buses_;
    }

    const std::deque<Stop> &TransportCatalogue::GetStopList() const
    {
        return stops_;
    }

    int TransportCatalogue::GetStopCount() const
    {
        return stops_.size();
    }
}

#include "transport_catalogue.h"

#include <algorithm>
#include <iomanip>
#include <set>
#include <unordered_set>

namespace catalogue
{
    namespace storage
    {
        void TransportCatalogue::AddStop(std::string_view name, Coordinates coords)
        {
            stops_.push_back({std::string(name), coords, {}});
            stopname_to_stop_.insert({std::hash<std::string_view>{}(name), &stops_.back()});
        }

        std::optional<std::string> TransportCatalogue::FindStop(std::string_view name) const
        {
            const int hash = std::hash<std::string_view>{}(name);
            if (stopname_to_stop_.find(hash) == stopname_to_stop_.end())
            {
                return std::nullopt;
            }
            return stopname_to_stop_.at(hash)->stop_name;
        }

        void TransportCatalogue::AddBus(std::string_view name, std::vector<std::string_view> stops)
        {
            std::deque<Stop *> bus_stops;
            const int bus_hash = std::hash<std::string_view>{}(name);
            for (const auto &stop : stops)
            {
                const int stop_hash = std::hash<std::string_view>{}(stop);
                bus_stops.push_back(stopname_to_stop_.at(stop_hash));
                stopname_to_stop_.at(stop_hash)->buses.push_back(bus_hash);
            }

            buses_.push_back({std::string(name), move(bus_stops)});
            busname_to_bus_.insert({bus_hash, &buses_.back()});
        }

        std::optional<std::string> TransportCatalogue::FindBus(const std::string_view name) const
        {
            const int hash = std::hash<std::string_view>{}(name);
            if (busname_to_bus_.find(hash) == busname_to_bus_.end())
            {
                return std::nullopt;
            }
            return busname_to_bus_.at(hash)->bus_name;
        }

        void TransportCatalogue::GetBusInfo(std::string_view name, std::ostream &output) const
        {
            if (FindBus(name))
            {
                const int hash = std::hash<std::string_view>{}(name);
                int stops_count = busname_to_bus_.at(hash)->bus_stops.size();

                std::unordered_set<std::string_view> unique_stop;
                for (const auto &stop : busname_to_bus_.at(hash)->bus_stops)
                {
                    unique_stop.insert(stop->stop_name);
                }

                double route_length = 0;
                for (auto iter = busname_to_bus_.at(hash)->bus_stops.begin(); iter != std::prev(busname_to_bus_.at(hash)->bus_stops.end());)
                {
                    const auto other_iter = iter;
                    ++iter;
                    route_length += ComputeDistance((*other_iter)->coords, (*iter)->coords);
                }

                output << "Bus " << name << ": " << stops_count << " stops on route, "
                       << unique_stop.size() << " unique stops, "
                       << std::setprecision(6) << route_length << " route length" << std::endl;
            }
            else
            {
                output << "Bus " << name << ": not found" << std::endl;
            }
        }

        void TransportCatalogue::GetStopInfo(std::string_view name, std::ostream &output) const
        {
            if (FindStop(name))
            {
                output << "Stop " << name;
                const int hash = std::hash<std::string_view>{}(name);
                if (stopname_to_stop_.at(hash)->buses.size() == 0)
                {
                    output << ": no buses" << std::endl;
                    return;
                }

                std::set<std::string_view> sorted_buses;
                for (const auto bus_hash : stopname_to_stop_.at(hash)->buses)
                {
                    sorted_buses.insert(busname_to_bus_.at(bus_hash)->bus_name);
                }

                output << ": buses";
                for (const auto bus : sorted_buses)
                {
                    output << " " << bus;
                }
                output << std::endl;
            }
            else
            {
                output << "Stop " << name << ": not found" << std::endl;
            }
        }
    }

}

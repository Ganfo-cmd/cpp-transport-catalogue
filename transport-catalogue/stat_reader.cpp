#include "stat_reader.h"

#include <iomanip>
#include <set>

namespace catalogue
{
    namespace detail
    {
        std::pair<std::string_view, std::string_view> ParseRequest(std::string_view request)
        {
            auto space_pos = request.find(' ');
            auto not_space = request.find_first_not_of(' ', space_pos);
            return {request.substr(0, space_pos),
                    request.substr(not_space)};
        }

        void PrintBusStat(const TransportCatalogue &transport_catalogue, std::string_view id, std::ostream &output)
        {
            BusInfo bus_info = transport_catalogue.GetBusInfo(id);
            if (bus_info.stops_count > 0)
            {
                output << "Bus " << id << ": " << bus_info.stops_count << " stops on route, "
                       << bus_info.unique_stops << " unique stops, "
                       << std::setprecision(6) << bus_info.route_length << " route length" << std::endl;
            }
            else
            {
                output << "Bus " << id << ": not found" << std::endl;
            }
        }

        struct BusPtrComparator
        {
            bool operator()(const Bus *left, const Bus *right) const
            {
                return left->bus_name < right->bus_name;
            }
        };

        void PrintStopStat(const TransportCatalogue &transport_catalogue, std::string_view id, std::ostream &output)
        {
            const auto buses = transport_catalogue.GetStopInfo(id);
            if (buses != nullptr)
            {
                output << "Stop " << id;
                if ((*buses).size() == 0)
                {
                    output << ": no buses" << std::endl;
                }
                else
                {
                    output << ": buses";
                    const std::set<const Bus *, BusPtrComparator> sorted_buses(buses->begin(), buses->end());
                    for (const auto bus : sorted_buses)
                    {
                        output << " " << bus->bus_name;
                    }
                    output << std::endl;
                }
            }
            else
            {
                output << "Stop " << id << ": not found" << std::endl;
            }
        }

        void ParseAndPrintStat(const TransportCatalogue &transport_catalogue, std::string_view request,
                               std::ostream &output)
        {
            const auto [command, id] = ParseRequest(request);
            if (command == "Bus")
            {
                PrintBusStat(transport_catalogue, id, output);
            }
            else if (command == "Stop")
            {
                PrintStopStat(transport_catalogue, id, output);
            }
        }
    }

}

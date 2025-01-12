#include "stat_reader.h"

#include <iomanip>

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

        void ParseAndPrintStat(const TransportCatalogue &transport_catalogue, std::string_view request,
                               std::ostream &output)
        {
            const auto [command, id] = ParseRequest(request);
            if (command == "Bus")
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
            else if (command == "Stop")
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
                        for (const auto bus : *buses)
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
        }
    }

}

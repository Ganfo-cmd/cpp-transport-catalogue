#include "stat_reader.h"

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

        void ParseAndPrintStat(const storage::TransportCatalogue &transport_catalogue, std::string_view request,
                               std::ostream &output)
        {
            const auto [command, id] = ParseRequest(request);
            if (command == "Bus")
            {
                transport_catalogue.GetBusInfo(id, output);
            }
            else if (command == "Stop")
            {
                transport_catalogue.GetStopInfo(id, output);
            }
        }
    }

}

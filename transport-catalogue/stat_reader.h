#pragma once

#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"

namespace catalogue
{
    namespace detail
    {
        void ParseAndPrintStat(const storage::TransportCatalogue &tansport_catalogue, std::string_view request,
                               std::ostream &output);
    }

}
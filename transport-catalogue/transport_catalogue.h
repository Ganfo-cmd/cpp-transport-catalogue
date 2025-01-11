#pragma once

#include "geo.h"

#include <deque>
#include <optional>
#include <unordered_map>
#include <string>
#include <vector>

namespace catalogue
{
	namespace storage
	{
		class TransportCatalogue
		{
		public:
			void AddStop(std::string_view name, Coordinates coords);

			std::optional<std::string> FindStop(std::string_view name) const;

			void AddBus(std::string_view name, std::vector<std::string_view> routes);

			std::optional<std::string> FindBus(std::string_view name) const;

			void GetBusInfo(std::string_view name, std::ostream &output) const;

			void GetStopInfo(std::string_view name, std::ostream &output) const;

		private:
			struct Stop
			{
				std::string stop_name;
				Coordinates coords;
				std::deque<int> buses;
			};

			struct Bus
			{
				std::string bus_name;
				std::deque<Stop *> bus_stops;
			};

			std::deque<Stop> stops_;
			std::unordered_map<int, Stop *> stopname_to_stop_;
			std::deque<Bus> buses_;
			std::unordered_map<int, Bus *> busname_to_bus_;
		};
	}

}

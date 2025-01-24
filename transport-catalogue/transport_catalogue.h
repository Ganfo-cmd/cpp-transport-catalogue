#pragma once

#include "geo.h"

#include <deque>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>

namespace catalogue
{
	struct Stop
	{
		std::string stop_name;
		Coordinates coords;
	};

	struct Bus
	{
		std::string bus_name;
		std::vector<const Stop *> bus_stops;
	};

	struct BusInfo
	{
		int stops_count = 0;
		int unique_stops = 0;
		double route_length = 0.0;
		double curvature = 0.0;
	};

	class TransportCatalogue
	{
	public:
		void AddStop(const std::string &name, Coordinates coords);

		const Stop *FindStop(std::string_view name) const;

		void AddBus(const std::string &name, const std::vector<std::string_view> &routes);

		const Bus *FindBus(std::string_view name) const;

		void SetDistance(const std::string &name, const std::vector<std::pair<double, std::string_view>> &other_stops);

		double GetDistance(const Stop *, const Stop *) const;

		BusInfo GetBusInfo(std::string_view name) const;

		const std::unordered_set<const Bus *> &GetStopInfo(std::string_view name) const;

	private:
		struct StopPtrHasher
		{
			size_t operator()(std::pair<const Stop *, const Stop *> const &stops_ptr) const
			{
				return std::hash<const Stop *>()(stops_ptr.first) * 37 + std::hash<const Stop *>()(stops_ptr.second) * 37 * 37;
			}
		};

		std::deque<Stop> stops_;
		std::unordered_map<std::string_view, const Stop *> stopname_to_stop_;
		std::unordered_map<std::pair<const Stop *, const Stop *>, double, StopPtrHasher> distances_by_stops_;
		std::unordered_map<std::string_view, std::unordered_set<const Bus *>> buses_by_stopname_;
		std::deque<Bus> buses_;
		std::unordered_map<std::string_view, const Bus *> busname_to_bus_;
	};
}

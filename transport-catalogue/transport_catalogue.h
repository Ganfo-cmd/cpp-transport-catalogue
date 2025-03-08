#pragma once

#include "domain.h"

#include <deque>
#include <optional>
#include <unordered_map>
#include <unordered_set>

namespace catalogue
{
	class TransportCatalogue
	{
	public:
		void AddStop(const std::string &name, geo::Coordinates coords);

		const Stop *FindStop(std::string_view name) const;

		void AddBus(const std::string &name, const std::vector<std::string_view> &routes, bool is_roundtrip);

		const Bus *FindBus(std::string_view name) const;

		void SetDistance(std::string_view stop_name, std::string_view other_stop, int distance);

		int GetDistance(const Stop *, const Stop *) const;

		BusInfo GetBusInfo(std::string_view name) const;

		const std::unordered_set<const Bus *> &GetStopInfo(std::string_view name) const;

		const std::deque<Bus> &GetBusList() const;

		const std::deque<Stop> &GetStopList() const;

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
		std::unordered_map<std::pair<const Stop *, const Stop *>, int, StopPtrHasher> distances_by_stops_;
		std::unordered_map<std::string_view, std::unordered_set<const Bus *>> buses_by_stopname_;
		std::deque<Bus> buses_;
		std::unordered_map<std::string_view, const Bus *> busname_to_bus_;
	};
}

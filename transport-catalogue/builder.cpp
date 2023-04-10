#include "builder.h"

#include <string>

#include "json_builder.h"

namespace builder {
    using namespace catalogue;
    using namespace std::literals;

    namespace {
        void BuildBusResponse(int request_id, const BusStat& statistics, json::Builder& json) {
            json.StartDict()
                .Key("request_id"s).Value(request_id)
                .Key("route_length"s).Value(statistics.route_length)
                .Key("curvature"s).Value(statistics.curvature)
                .Key("stop_count"s).Value(static_cast<int>(statistics.stops_count))
                .Key("unique_stop_count"s).Value(static_cast<int>(statistics.unique_stops_count))
                .EndDict();
        }

        void BuildStopResponse(int request_id, const std::set<std::string_view>& buses, json::Builder& json) {
            json.StartDict()
                .Key("request_id"s).Value(request_id)
                .Key("buses"s).StartArray();
            for (std::string_view bus : buses) {
                json.Value(std::string(bus));
            }
            json.EndArray()
                .EndDict();
        }

        void BuildErrorResponse(int request_id, json::Builder& json) {
            json.StartDict()
                .Key("request_id"s).Value(request_id)
                .Key("error_message"s).Value("not found"s)
                .EndDict();
        }

        void BuildMapResponse(int request_id, const std::string& image, json::Builder& json) {
            json.StartDict().Key("request_id"s).Value(request_id).Key("map"s).Value(image).EndDict();
        }

    }  // namespace

    json::Node BuildResponse(const TransportCatalogue& catalogue, const json::Array& requests, const renderer::Visualization& settings) {
        auto json = json::Builder();
        json.StartArray();

        for (const auto& request : requests) {
            int id = request.AsDict().at("id"s).AsInt();

            if (request.AsDict().at("type"s).AsString() == "Bus"s) {
                if (auto bus_statistics = catalogue.GetBusStat(request.AsDict().at("name"s).AsString())) {
                    BuildBusResponse(id, *bus_statistics, json);
                } else {
                    BuildErrorResponse(id, json);
                }
            } else if (request.AsDict().at("type"s).AsString() == "Stop"s) {
                if (auto buses = catalogue.GetBusPassStop(request.AsDict().at("name"s).AsString())) {
                    BuildStopResponse(id, *buses, json);
                } else {
                    BuildErrorResponse(id, json);
                }
            } else if (request.AsDict().at("type"s).AsString() == "Map"s) {
                BuildMapResponse(id, RenderTransportMap(catalogue, settings), json);
            }
        }

        return std::move(json.EndArray().Build());
    }
}  // namespace builder
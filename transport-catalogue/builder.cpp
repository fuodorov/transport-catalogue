#include "builder.h"

#include <string>

#include "json_builder.h"

namespace builder {
    using namespace catalogue;
    using namespace std::literals;

    namespace {
        void BuildBusResponse(int request_id, const BusStat& statistics, json::Builder& response) {
            response.StartDict();
            response.Key("curvature"s).Value(statistics.curvature);
            response.Key("request_id"s).Value(request_id);
            response.Key("route_length"s).Value(statistics.route_length);
            response.Key("stop_count"s).Value(static_cast<int>(statistics.stops_count));
            response.Key("unique_stop_count"s).Value(static_cast<int>(statistics.unique_stops_count));
            response.EndDict();
        }

        void BuildStopResponse(int request_id, const std::set<std::string_view>& buses, json::Builder& response) {
            response.StartDict();
            response.Key("request_id"s).Value(request_id);

            response.Key("buses"s).StartArray();
            for (std::string_view bus : buses)
                response.Value(std::string(bus));
            response.EndArray();

            response.EndDict();
        }

        void BuildErrorResponse(int request_id, json::Builder& response) {
            response.StartDict();
            response.Key("request_id"s).Value(request_id);
            response.Key("error_message"s).Value("not found"s);
            response.EndDict();
        }

        void BuildMapResponse(int request_id, const std::string& image, json::Builder& response) {
            response.StartDict();
            response.Key("request_id"s).Value(request_id);
            response.Key("map"s).Value(image);
            response.EndDict();
        }

    }  // namespace

    json::Node BuildResponse(const TransportCatalogue& catalogue, const json::Array& requests, const renderer::Visualization& settings) {
        auto response = json::Builder();
        response.StartArray();

        for (const auto& request : requests) {
            const auto& request_dict_view = request.AsDict();

            int request_id = request_dict_view.at("id"s).AsInt();
            std::string type = request_dict_view.at("type"s).AsString();
            std::string name;  //> Could be a name of bus or a stop

            if (type == "Bus"s) {
                name = request_dict_view.at("name"s).AsString();

                if (auto bus_statistics = catalogue.GetBusStat(name)) {
                    BuildBusResponse(request_id, *bus_statistics, response);
                } else {
                    BuildErrorResponse(request_id, response);
                }
            } else if (type == "Stop"s) {
                name = request_dict_view.at("name"s).AsString();
                if (auto buses = catalogue.GetBusesPassingThroughTheStop(name)) {
                    BuildStopResponse(request_id, *buses, response);
                } else {
                    BuildErrorResponse(request_id, response);
                }
            } else if (type == "Map"s) {
                std::string image = RenderTransportMap(catalogue, settings);
                BuildMapResponse(request_id, image, response);
            }
        }

        response.EndArray();
        return std::move(response.Build());
    }
}  // namespace builder
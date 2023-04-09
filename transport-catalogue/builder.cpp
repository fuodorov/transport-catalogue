#include "builder.h"

#include <string>

namespace builder {
    using namespace catalogue;
    using namespace std::literals;

    namespace {
        json::Node BuildBusResponse(int request_id, const BusStat& statistics) {
            json::Dict response;
            response.emplace("curvature"s, statistics.curvature);
            response.emplace("request_id"s, request_id);
            response.emplace("route_length"s, statistics.route_length);
            response.emplace("stop_count"s, static_cast<int>(statistics.stops_count));
            response.emplace("unique_stop_count"s, static_cast<int>(statistics.unique_stops_count));
            return response;
        }

        json::Node BuildStopResponse(int request_id, const std::set<std::string_view>& buses) {
            json::Array buses_array;
            buses_array.reserve(buses.size());
            for (std::string_view bus : buses) {
                buses_array.emplace_back(std::string(bus));
            }
            return json::Dict{{"request_id"s, request_id}, {"buses"s, std::move(buses_array)}};
        }

        json::Node BuildErrorResponse(int request_id) {
            return json::Dict{{"request_id"s, request_id}, {"error_message"s, "not found"s}};
        }

        json::Node BuildMapResponse(int request_id, const std::string& image) {
            return json::Dict{{"request_id"s, request_id}, {"map"s, image}};
        }

    }  // namespace

    json::Node BuildResponse(const TransportCatalogue& catalogue, const json::Array& requests,
                                const renderer::Visualization& settings) {
        json::Array response;
        response.reserve(requests.size());

        for (const auto& request : requests) {
            const auto& request_dict_view = request.AsMap();

            int request_id = request_dict_view.at("id"s).AsInt();
            std::string type = request_dict_view.at("type"s).AsString();
            std::string name;

            if (type == "Bus"s) {
                name = request_dict_view.at("name"s).AsString();

                if (auto bus_statistics = catalogue.GetBusStat(name)) {
                    response.emplace_back(BuildBusResponse(request_id, *bus_statistics));
                } else {
                    response.emplace_back(BuildErrorResponse(request_id));
                }
            } else if (type == "Stop"s) {
                name = request_dict_view.at("name"s).AsString();
                if (auto buses = catalogue.GetBusesPassingThroughTheStop(name)) {
                    response.emplace_back(BuildStopResponse(request_id, *buses));
                } else {
                    response.emplace_back(BuildErrorResponse(request_id));
                }
            } else if (type == "Map"s) {
                std::string image = RenderTransportMap(catalogue, settings);
                response.emplace_back(BuildMapResponse(request_id, image));
            }
        }

        return response;
    }
}  // namespace builder
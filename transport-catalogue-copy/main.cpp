#include <string>

#include "json_reader.h"

using namespace catalogue;
using namespace std::literals;

void JsonQueriesEngine(std::istream& input, std::ostream& output) {
    const auto json = json::Load(input).GetRoot();
    TransportCatalogue catalogue;

    auto response = request::MakeStatResponse(
        request::ProcessBaseRequest(json.AsMap().at("base_requests"s).AsArray()), 
        json.AsMap().at("stat_requests"s).AsArray(), 
        request::ParseVisualizationSettings(json.AsMap().at("render_settings"s).AsMap())
    );

    json::Print(json::Document{std::move(response)}, output);
}


int main() {
    JsonQueriesEngine(std::cin, std::cout);
    return 0;
}
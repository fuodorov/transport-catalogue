#include <string>

#include "parser.h"

void JsonQueriesEngine(std::istream& input, std::ostream& output) {
    using namespace catalogue;
    using namespace std::literals;

    const auto json = json::Load(input).GetRoot();
    TransportCatalogue catalogue;

    auto response = parser::MakeStatResponse(
        parser::ParseQueries(json.AsMap().at("base_requests"s).AsArray()), 
        json.AsMap().at("stat_requests"s).AsArray(), 
        parser::ParseRenderSettings(json.AsMap().at("render_settings"s).AsMap())
    );

    json::Print(json::Document{std::move(response)}, output);
}


int main() {
    JsonQueriesEngine(std::cin, std::cout);
    return 0;
}
#include <string>

#include "parser.h"
#include "builder.h"

void QueriesEngine(std::istream& input, std::ostream& output) {
    using namespace std::literals;
    const auto json = json::Load(input).GetRoot();
    auto response = builder::BuildResponse(parser::ParseQueries(json.AsDict().at("base_requests"s).AsArray()),  json.AsDict().at("stat_requests"s).AsArray(), parser::ParseRenderSettings(json.AsDict().at("render_settings"s).AsDict()));
    json::Print(json::Document{std::move(response)}, output);
}

int main() {
    QueriesEngine(std::cin, std::cout);
    return 0;
}
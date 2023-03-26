#include "parser.h"

#include <cassert>
#include <regex>


namespace catalogue::parser {
    using namespace std::literals;
    using namespace catalogue;

    std::string_view RemoveFirstWord(std::string_view text) {
        size_t word_end = text.find(" "sv);
        return text.substr(word_end + (" "sv).size(), text.size() - word_end);
    }

    Distances ParseDistances(std::string_view text) {
        Distances result;

        std::regex distance_regex(R"(, ([\d.]+)m to ([\w ]+))");
        std::smatch match;
        std::string text_copy = text.data();
        
        int math_initial_position = 0;
        while (std::regex_search(text_copy, match, distance_regex)) {
            std::string_view stop_to = text.substr(math_initial_position + match.position(2), match.length(2));
            int distance = std::stoi(std::string(match[1]));
            result.emplace_back(stop_to, distance);
            text_copy = match.suffix();
            math_initial_position += match.position(0) + match.length(0);
        }

        return result;
    }

    std::pair<catalogue::Stop, bool> ParseCoordinates(const std::string& text) {
        Stop stop;

        std::regex stop_regex(R"(Stop ([\w ]+)?: ([\d.]+), ([\d.]+)(, [\d.]+m to [\w ]+)*$)");
        std::smatch match;
        std::regex_match(text, match, stop_regex);
        
        stop.name = match[1];
        stop.point.lat = std::stod(match[2]);
        stop.point.lng = std::stod(match[3]);

        return {std::move(stop), match[4].matched};
    }

    std::vector<std::string_view> Split(std::string_view text, std::string_view separator) {
        std::vector<std::string_view> result;

        size_t word_begin = 0;
        while (word_begin < text.size()) {
            size_t word_end = text.find(separator, word_begin);
            result.push_back(text.substr(word_begin, word_end - word_begin));
            word_begin = (word_end == std::string_view::npos) ? word_end : word_end + separator.size();
        }

        return result;
    }

    Bus ParseRoutes(std::string_view text) {
        Bus result;

        result.number = RemoveFirstWord(text.substr(0, text.find(": "sv)));
        result.type = (text.find(" > "sv) != std::string_view::npos) ? RouteType::CIRCLE : RouteType::TWO_DIRECTIONAL;
        result.stops = Split(text.substr(text.find(": "sv) + (": "sv).size()), (result.type == RouteType::CIRCLE) ? " > "sv : " - "sv);
        result.unique_stops = {result.stops.begin(), result.stops.end()};

        return result;
    }

    void FillQueries(TransportCatalogue &catalogue, std::istream &input_stream, int count, std::vector<std::string> &queries, std::vector<std::pair<std::string, std::string>> &distances) {
        std::string query;
        for (int i = 0; i < count; ++i) {
            std::getline(input_stream, query);
            if (query.substr(0, 4) == "Stop"s) {
                auto [stop, is_store_query] = ParseCoordinates(query);
                if (is_store_query) {
                    distances.emplace_back(stop.name, std::move(query));
                }
                catalogue.AddStop(std::move(stop));
            } else if (query.substr(0, 3) == "Bus"s) {
                queries.emplace_back(std::move(query));
            }
        }
    }

    void ParseQueries(TransportCatalogue &catalogue, std::istream &input_stream, std::ostream &output_stream) {
        int count{0};
        input_stream >> count;
        input_stream.get();

        std::vector<std::string> queries;
        std::vector<std::pair<std::string, std::string>> distances;
        queries.reserve(count);
        distances.reserve(count);

        FillQueries(catalogue, input_stream, count, queries, distances);

        for (const auto& [from, query] : distances) {
            for (auto [to, distance] : ParseDistances(query))
                catalogue.AddDistance(from, to, distance);
        }

        for (const auto& query : queries) {
            catalogue.AddBus(ParseRoutes(query));
        }

        input_stream >> count;
        input_stream.get();
        std::string query;
        for (int i = 0; i < count; ++i) {
            std::getline(input_stream, query);
            std::string_view info = RemoveFirstWord(query);
            if (query.substr(0, 3) == "Bus"s) {
                if (auto statistics = catalogue.GetBusStat(info)) {
                    output_stream << *statistics << std::endl;
                } else {
                    output_stream << "Bus " << info << ": not found" << std::endl;
                }
            } else if (query.substr(0, 4) == "Stop"s) {
                auto* buses = catalogue.GetBusPassStop(info);

                if (!buses) {
                    output_stream << "Stop " << info << ": not found" << std::endl;
                } else if (buses->empty()) {
                    output_stream << "Stop " << info << ": no buses" << std::endl;
                } else {
                    output_stream << "Stop " << info << ": buses";
                    for (const auto& bus : *buses) {
                        output_stream << " " << bus;
                    }
                    output_stream << std::endl;
                }
            }
        }
    }

}  // namespace catalogue::parser
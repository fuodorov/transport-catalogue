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

    DistancesToStops ParseDistances(std::string_view text) {
        DistancesToStops result;

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

    Bus ParseRoutes(std::string_view text) {
        //! Input format for circle route: Bus Y: Stop#1 > Stop#2 > Stop#3 ..
        //! Input format for two-directional route: Bus Y: Stop#1 - Stop#2 - Stop#3 ..

        Bus result;

        size_t bus_start = text.find(' ') + (" "sv).size();
        size_t bus_end = text.find(": "sv, bus_start);
        result.number = text.substr(bus_start, bus_end - bus_start);

        result.type = (text[text.find_first_of("->")] == '>') ? RouteType::CIRCLE : RouteType::TWO_DIRECTIONAL;
        std::string_view stops_separator = (result.type == RouteType::CIRCLE) ? " > "sv : " - "sv;

        size_t stop_begin = bus_end + (": "sv).size();
        while (stop_begin <= text.length()) {
            size_t stop_end = text.find(stops_separator, stop_begin);

            result.stop_names.push_back(text.substr(stop_begin, stop_end - stop_begin));
            stop_begin = (stop_end == std::string_view::npos) ? stop_end : stop_end + stops_separator.size();
        }

        result.unique_stops = {result.stop_names.begin(), result.stop_names.end()};

        return result;
    }

    void ParseQueries(TransportCatalogue &catalogue, std::istream &input_stream, std::ostream &output_stream) {
        int queries_count{0};
        input_stream >> queries_count;
        input_stream.get();

        std::vector<std::string> bus_queries;
        bus_queries.reserve(queries_count);
        std::vector<std::pair<std::string, std::string>> stop_distances;
        stop_distances.reserve(queries_count);

        std::string query;
        for (int id = 0; id < queries_count; ++id) {
            std::getline(input_stream, query);
            if (query.substr(0, 4) == "Stop"s) {
                auto [stop, is_store_query] = ParseCoordinates(query);
                if (is_store_query)
                    stop_distances.emplace_back(stop.name, std::move(query));
                catalogue.AddStop(std::move(stop));
            } else if (query.substr(0, 3) == "Bus"s) {
                bus_queries.emplace_back(std::move(query));
            }
        }

        for (const auto& [stop_from, query] : stop_distances) {
            for (auto [stop_to, distance] : ParseDistances(query))
                catalogue.AddDistance(stop_from, stop_to, distance);
        }

        for (const auto& bus_query : bus_queries)
            catalogue.AddBus(ParseRoutes(bus_query));

        input_stream >> queries_count;
        input_stream.get();
        for (int id = 0; id < queries_count; ++id) {
            std::getline(input_stream, query);
            if (query.substr(0, 3) == "Bus"s) {
                std::string_view bus_number = RemoveFirstWord(query);
                
                if (auto bus_statistics = catalogue.GetBusStatistics(bus_number)) {
                    output_stream << *bus_statistics << std::endl;
                } else {
                    output_stream << "Bus " << bus_number << ": not found" << std::endl;
                }
            } else if (query.substr(0, 4) == "Stop"s) {
                std::string_view stop_name = RemoveFirstWord(query);
                auto* buses = catalogue.GetBusesPassingThroughTheStop(stop_name);

                if (!buses) {
                    output_stream << "Stop " << stop_name << ": not found" << std::endl;
                } else if (buses->empty()) {
                    output_stream << "Stop " << stop_name << ": no buses" << std::endl;
                } else {
                    output_stream << "Stop " << stop_name << ": buses";
                    for (const auto& bus : *buses)
                        output_stream << " " << bus;
                    output_stream << std::endl;
                }
            }
        }
    }

}  // namespace catalogue::parser
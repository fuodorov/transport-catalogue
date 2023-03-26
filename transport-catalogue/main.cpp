// #include <cassert>
// #include <sstream>

// #include "geo.h"
// #include "input_reader.h"
// #include "stat_reader.h"
// #include "transport_catalogue.h"

// using namespace std;
// using namespace transport_catalogue;

// void QueriesEngine(std::istream& input, std::ostream& output) {
//     TransportCatalogue transport_catalogue;
//     int query_count = ReadLineWithNumber(input);
//     vector<string> queries;
//     queries.reserve(query_count);
//     for (int i = 0; i < query_count; ++i) {
//         queries.push_back(ReadLine(input));
//     }
//     InputQueries(transport_catalogue, queries);
//     int stat_query_count = ReadLineWithNumber(input);
//     for (int i = 0; i < stat_query_count; ++i) {
//         OutputQuery(transport_catalogue, ReadLine(input), output);
//     }
// }

// void TestQueriesEngine(stringstream& input, stringstream& output) {
//     stringstream result;
//     QueriesEngine(input, result);
//     string result_line, output_line;
//     while (getline(result, result_line) && getline(output, output_line)) {
//         if (result_line != output_line) {
//             cout << "TestQueriesEngine failed" << endl;
//             cout << "Expected: " << output_line << endl;
//             cout << "Actual: " << result_line << endl;
//             return;
//         }
//     }
//     cout << "TestQueriesEngine OK" << endl;
// }

// int main() {
//     const vector<string> input = {
//         "13\n"s,
//         "Stop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino\n"s,
//         "Stop Marushkino: 55.595884, 37.209755, 9900m to Rasskazovka, 100m to Marushkino\n"s,
//         "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\n"s,
//         "Bus 750: Tolstopaltsevo - Marushkino - Marushkino - Rasskazovka\n"s,
//         "Stop Rasskazovka: 55.632761, 37.333324, 9500m to Marushkino\n"s,
//         "Stop Biryulyovo Zapadnoye: 55.574371, 37.6517, 7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam\n"s,
//         "Stop Biryusinka: 55.581065, 37.64839, 750m to Universam\n"s,
//         "Stop Universam: 55.587655, 37.645687, 5600m to Rossoshanskaya ulitsa, 900m to Biryulyovo Tovarnaya\n"s,
//         "Stop Biryulyovo Tovarnaya: 55.592028, 37.653656, 1300m to Biryulyovo Passazhirskaya\n"s,
//         "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164, 1200m to Biryulyovo Zapadnoye\n"s,
//         "Bus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye\n"s,
//         "Stop Rossoshanskaya ulitsa: 55.595579, 37.605757\n"s,
//         "Stop Prazhskaya: 55.611678, 37.603831\n"s,
//         "6\n"s,
//         "Bus 256\n"s,
//         "Bus 750\n"s,
//         "Bus 751\n"s,
//         "Stop Samara\n"s,
//         "Stop Prazhskaya\n"s,
//         "Stop Biryulyovo Zapadnoye\n"s
//     };
//     const vector<string> output = {
//         "Bus 256: 6 stops on route, 5 unique stops, 5950 route length, 1.36124 curvature\n"s,
//         "Bus 750: 7 stops on route, 3 unique stops, 27400 route length, 1.30853 curvature\n"s,
//         "Bus 751: not found\n"s,
//         "Stop Samara: not found\n"s,
//         "Stop Prazhskaya: no buses\n"s,
//         "Stop Biryulyovo Zapadnoye: buses 256 828\n"s
//     };
//     stringstream input_ss, output_ss;
//     for (const auto& line : input) {
//         input_ss << line;
//     }
//     for (const auto& line : output) {
//         output_ss << line;
//     }
//     TestQueriesEngine(input_ss, output_ss);
//     return 0;
// }

#include <fstream>
#include <iostream>

#include "input_reader.h"

using namespace std::literals;
using namespace catalog::input_utils;

int main() {
    ParseTransportCatalogueQueries(std::cin);
    return 0;
}
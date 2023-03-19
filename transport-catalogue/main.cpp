#include "geo.h"
#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

using namespace std;
using namespace transport_catalogue;

void QueriesEngine(std::istream& input, std::ostream& output) {
    TransportCatalogue transport_catalogue;
    int query_count = ReadLineWithNumber(input);
    vector<string> queries;
    queries.reserve(query_count);
    for (int i = 0; i < query_count; ++i) {
        queries.push_back(ReadLine(input));
    }
    InputQueries(transport_catalogue, queries);
    int stat_query_count = ReadLineWithNumber(input);
    for (int i = 0; i < stat_query_count; ++i) {
        OutputQuery(transport_catalogue, ReadLine(input), output);
    }
}

int main() {
    QueriesEngine(std::cin, std::cout);
    return 0;
}

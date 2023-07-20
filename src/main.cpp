#include <fstream>
#include <iostream>

#include "handler.h"
#include "reader.h"

using namespace std;

using namespace transport_catalogue;
using namespace transport_catalogue::detail::json;
using namespace transport_catalogue::detail::router;

using namespace map_renderer;
using namespace request_handler;
using namespace serialization;

void PrintUsage(std::ostream &stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    TransportCatalogue transport_catalogue;

    RenderSettings render_settings;
    RoutingSettings routing_settings;

    SerializationSettings serialization_settings;

    JSONReader json_reader;
    vector<StatRequest> stat_request;

    if (mode == "make_base"sv) {
        json_reader = JSONReader(cin);

        json_reader.parse_node_make_base(transport_catalogue, render_settings, routing_settings,
                                         serialization_settings);

        ofstream out_file(serialization_settings.file_name, ios::binary);
        catalogue_serialization(transport_catalogue, render_settings, routing_settings, out_file);

    } else if (mode == "process_requests"sv) {
        json_reader = JSONReader(cin);

        json_reader.parse_node_process_requests(stat_request, serialization_settings);

        ifstream in_file(serialization_settings.file_name, ios::binary);

        Catalogue catalogue = catalogue_deserialization(in_file);

        RequestHandler request_handler;

        request_handler.execute_queries(catalogue.transport_catalogue_, stat_request, catalogue.render_settings_,
                                        catalogue.routing_settings_);

        print(request_handler.get_document(), cout);

    } else {
        PrintUsage();
        return 1;
    }
}
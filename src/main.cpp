#include <fstream>
#include <iostream>

#include "handler.h"
#include "reader.h"

using namespace std;

using namespace transport_catalogue;
using namespace transport_catalogue::json;
using namespace transport_catalogue::router;

using namespace renderer;
using namespace handler;
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

  Parser json_reader;
  vector<StatisticRequest> stat_request;

  if (mode == "make_base"sv) {
    json_reader = Parser(cin);

    json_reader.ProcessTransportCatalogue(transport_catalogue, render_settings,
                                          routing_settings,
                                          serialization_settings);

    ofstream out_file(serialization_settings.file_name, ios::binary);
    CatalogueSerialization(transport_catalogue, render_settings,
                            routing_settings, out_file);

  } else if (mode == "process_requests"sv) {
    json_reader = Parser(cin);

    json_reader.ProcessRequests(stat_request, serialization_settings);

    ifstream in_file(serialization_settings.file_name, ios::binary);

    Catalogue catalogue = CatalogueDeserialization(in_file);

    Handler handler;

    handler.Queries(catalogue.transport_catalogue_, stat_request,
                    catalogue.render_settings_, catalogue.routing_settings_);

    Print(handler.GetDocument(), cout);

  } else {
    PrintUsage();
    return 1;
  }
}
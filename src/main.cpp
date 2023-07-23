#include <fstream>
#include <iostream>

#include "handler.h"
#include "log/easylogging++.h"
#include "reader.h"

using namespace std;
using namespace handler;
using namespace renderer;
using namespace serialization;
using namespace transport_catalogue;
using namespace transport_catalogue::json;
using namespace transport_catalogue::router;

INITIALIZE_EASYLOGGINGPP

void PrintUsage(std::ostream &stream = std::cerr) {
  stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char *argv[]) {
  LOG(INFO) << "Start program";

  if (argc != 2) {
    LOG(ERROR) << "Invalid arguments count"sv;
    PrintUsage();
    return 1;
  }

  const std::string_view mode(argv[1]);
  TransportCatalogue transport_catalogue;
  RenderSettings render_settings;
  RoutingSettings routing_settings;
  SerializationSettings serialization_settings;
  vector<StatisticRequest> stat_request;

  if (mode == "make_base"sv) {
    LOG(INFO) << "Start make_base"sv;
    Parser(cin).ProcessTransportCatalogue(transport_catalogue, render_settings,
                                          routing_settings,
                                          serialization_settings);
    LOG(INFO) << "Start serialization"sv;
    ofstream file(serialization_settings.file_name, ios::binary);
    CatalogueSerialization(transport_catalogue, render_settings,
                           routing_settings, file);
    LOG(INFO) << "Save to file "sv << serialization_settings.file_name;
    LOG(INFO) << "End serialization"sv;
  } else if (mode == "process_requests"sv) {
    LOG(INFO) << "Start process_requests"sv;
    Parser(cin).ProcessRequests(stat_request, serialization_settings);
    ifstream file(serialization_settings.file_name, ios::binary);
    LOG(INFO) << "Start deserialization from file "sv
              << serialization_settings.file_name;
    Catalogue catalogue = CatalogueDeserialization(file);
    LOG(INFO) << "End deserialization"sv;
    Handler handler;
    handler.Queries(catalogue.transport_catalogue_, stat_request,
                    catalogue.render_settings_, catalogue.routing_settings_);
    Print(handler.GetDocument(), cout);
    LOG(INFO) << "End process_requests"sv;
  } else {
    LOG(ERROR) << "Invalid mode "sv << mode;
    PrintUsage();
    return 1;
  }
}
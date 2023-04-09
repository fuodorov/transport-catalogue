#include "json_reader.h"
#include "request_handler.h"

using namespace catalogue;

int main() {
    request::ProcessTransportCatalogueQuery(std::cin, std::cout);
    return 0;
}
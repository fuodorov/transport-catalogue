#include "json_reader.h"
#include "request_handler.h"

int main() {
  request::ProcessTransportCatalogueQuery(std::cin, std::cout);
  return 0;
}

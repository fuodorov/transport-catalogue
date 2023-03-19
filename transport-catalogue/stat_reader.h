#pragma once

#include <iostream>
#include <ostream>
#include <string>

#include "transport_catalogue.h"

void OutputQuery(const transport_catalogue::TransportCatalogue& transport_catalogue, const std::string& query, std::ostream& out = std::cout);

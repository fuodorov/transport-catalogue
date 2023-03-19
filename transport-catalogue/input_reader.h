#pragma once

#include <istream>
#include <string>
#include <vector>

#include "transport_catalogue.h"

std::string ReadLine(std::istream& input);

int ReadLineWithNumber(std::istream& input);

void InputQueries(transport_catalogue::TransportCatalogue& transport_catalogue, const std::vector<std::string>& queries);

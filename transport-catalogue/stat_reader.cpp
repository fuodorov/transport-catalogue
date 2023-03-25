#include <algorithm>

#include "stat_reader.h"

void OutputQuery(const transport_catalogue::TransportCatalogue& transport_catalogue, const std::string& query, std::ostream& out) {
    size_t space_pos = query.find_first_of(' ');
    std::string type = query.substr(0, space_pos);
    std::string name = query.substr(space_pos + 1);
    
    if (type == std::string("Bus")) {
        auto [info, is_found] = transport_catalogue.GetBus(name);

        if (is_found) {
            out << *(*info).second << std::endl;
        }
        else {
            out << std::string("Bus ") << name << std::string(": not found") << std::endl;
        }
    }
    else if (type == std::string("Stop")) {
        using namespace transport_catalogue::stop_catalogue;

        auto [info, is_found] = transport_catalogue.GetBusesForStop(name);

        if (is_found) {
            if (!info.empty()) {
                out << std::string("Stop ") << name << std::string(": buses ") << info << std::endl;
            }
            else {
                out << std::string("Stop ") << name << std::string(": no buses") << std::endl;
            }
        }
        else {
            out << std::string("Stop ") << name << std::string(": not found") << std::endl;
        }
    }
    else {
        throw std::invalid_argument("Unknown query type: " + type);
    }
}

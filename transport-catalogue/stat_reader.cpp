#include "stat_reader.h"

#include <cassert>
#include <iomanip>

namespace catalogue::output {

    using namespace std::literals;

    std::string_view ParseBusStatistics(std::string_view text) {
        //! Input: Bus BusNumber
        size_t bus_begin = text.find(" "sv) + (" "sv).size();
        return text.substr(bus_begin, text.size() - bus_begin);
    }

    std::string_view ParseBusPassStop(std::string_view text) {
        //! Input: Stop StopName
        size_t stop_begin = text.find(" "sv) + (" "sv).size();
        return text.substr(stop_begin, text.size() - stop_begin);
    }

    void PrintBusPassStop(std::ostream& os, std::string_view stop_name,
                                    const std::set<std::string_view>* buses) {
        if (!buses) {
            os << "Stop " << stop_name << ": not found" << std::endl;
        } else if (buses->empty()) {
            os << "Stop " << stop_name << ": no buses" << std::endl;
        } else {
            os << "Stop " << stop_name << ": buses ";
            size_t index{0u};
            for (std::string_view bus : *buses) {
                if (index++ != 0)
                    os << " ";
                os << bus;
            }
            os << std::endl;
        }
    }

}  // namespace catalogue::output
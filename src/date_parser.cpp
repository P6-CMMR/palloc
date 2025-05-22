#include "date_parser.hpp"

#include <spanstream>

using namespace palloc;

Uint DateParser::parseTimeToMinutes(std::string_view startTimeStr) {
    std::ispanstream iss{startTimeStr};
    std::chrono::minutes minutes;
    iss >> std::chrono::parse("%R", minutes);
    return static_cast<Uint>(minutes.count());
}
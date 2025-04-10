#include "date_parser.hpp"

#include <spanstream>

using namespace palloc;

uint32_t DateParser::parseTimeToMinutes(const std::string_view &startTimeStr) {
    std::ispanstream iss{startTimeStr};
    std::chrono::minutes minutes;
    iss >> std::chrono::parse("%R", minutes);
    return static_cast<uint32_t>(minutes.count());
}
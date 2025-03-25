#include "date_parser.hpp"

using namespace palloc;

uint64_t DateParser::parseTimeToMinutes(std::string &startTimeStr) {
    std::istringstream is{startTimeStr};
    std::chrono::minutes minutes;
    is >> std::chrono::parse("%R", minutes);
    return static_cast<uint64_t>(minutes.count());
}
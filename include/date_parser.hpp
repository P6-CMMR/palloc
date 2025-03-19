#ifndef DATE_PARSER_HPP
#define DATE_PARSER_HPP

#include <chrono>
#include <cstdint>
#include <sstream>
#include <string>

namespace palloc {
class DateParser {
   public:
    static uint64_t parseTimeToMinutes(std::string &startTimeStr);
};
}  // namespace palloc

#endif
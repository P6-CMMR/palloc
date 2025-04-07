#ifndef DATE_PARSER_HPP
#define DATE_PARSER_HPP

#include <chrono>
#include <cstdint>
#include <sstream>
#include <string_view>

namespace palloc {
class DateParser {
   public:
    static uint64_t parseTimeToMinutes(const std::string_view &startTimeStr);
};
}  // namespace palloc

#endif
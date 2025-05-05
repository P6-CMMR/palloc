#ifndef DATE_PARSER_HPP
#define DATE_PARSER_HPP

#include <chrono>
#include <sstream>
#include <string_view>

#include "types.hpp"

namespace palloc {
class DateParser {
   public:
    static Uint parseTimeToMinutes(const std::string_view &startTimeStr);
};
}  // namespace palloc

#endif
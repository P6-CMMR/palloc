#ifndef TYPES_HPP
#define TYPES_HPP

#include <cstdint>
#include <vector>

namespace palloc {
using Uint = uint32_t;
using Uint64 = uint64_t;
using Int = int32_t;
using UintVector = std::vector<Uint>;
using DoubleVector = std::vector<double>;
}  // namespace palloc

#endif
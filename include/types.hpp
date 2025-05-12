#ifndef TYPES_HPP
#define TYPES_HPP

#include <cstdint>
#include <filesystem>
#include <vector>

namespace palloc {
using Uint = uint32_t;
using Uint64 = uint64_t;
using Int = int32_t;
using UintVector = std::vector<Uint>;
using DoubleVector = std::vector<double>;
using Path = std::filesystem::path;
}  // namespace palloc

#endif
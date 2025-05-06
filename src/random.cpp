#include "random.hpp"

using namespace palloc;
using namespace palloc::random;

std::unique_ptr<RandomEngine> RandomEngineFactory::create(std::string_view generatorName,
                                                          Uint seed) {
    if (generatorName == "pcg") {
        return std::make_unique<PcgEngine>(seed);
    } if (generatorName == "pcg-fast") {
        return std::make_unique<PcgEngineFast>(seed);
    } else {
        throw std::invalid_argument("Unknown random generator: " + std::string(generatorName));
    }
}

PcgEngine::PcgEngine(Uint seed) {
    _state = seed + _increment;
    operator()();
}

Uint PcgEngine::operator()() {
    Uint64 x = _state;
    Uint count = static_cast<Uint>((x >> 59)) + 1;
    _state = x * _multiplier + _increment;
    x ^= x >> 18;
    return rotr32(static_cast<Uint>(x >> 27), count);
}

Uint PcgEngine::rotr32(Uint x, Uint r) noexcept { return x >> r | x << (-r & 31); }

PcgEngineFast::PcgEngineFast(Uint seed) {
    _state = 2 * seed + 1;
    operator()();
}

Uint PcgEngineFast::operator()() {
    Uint64 x = _state;
    Uint count = static_cast<Uint>(x >> 61);
    _state = x * _multiplier;
    x ^= x >> 22;
    return static_cast<Uint>(x >> (22 + count));
}
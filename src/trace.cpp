#include "trace.hpp"

using namespace palloc;

size_t Trace::getTimeStep() const noexcept { return timestep; }

size_t Trace::getNumberOfRequests() const noexcept { return numberOfRequests; }

size_t Trace::getNumberOfOngoingSimulations() const noexcept { return numberOfOngoingSimulations; }

size_t Trace::getAvailableParkingSpots() const noexcept { return availableParkingSpots; }

std::ostream &palloc::operator<<(std::ostream &os, const Trace &trace) {
    os << "Trace(time=" << trace.getTimeStep() << ", requests=" << trace.getNumberOfRequests()
       << ", simulations=" << trace.getNumberOfOngoingSimulations()
       << ", parkingSpots=" << trace.getAvailableParkingSpots() << ")";
    return os;
}

void Traces::saveToFile(const std::filesystem::path &outputPath) const {}

#include "trace.hpp"

using namespace palloc;

uint64_t Trace::getTimeStep() const noexcept { return timestep; }

size_t Trace::getNumberOfRequests() const noexcept { return numberOfRequests; }

size_t Trace::getNumberOfOngoingSimulations() const noexcept { return numberOfOngoingSimulations; }

size_t Trace::getAvailableParkingSpots() const noexcept { return availableParkingSpots; }

double Trace::getCost() const noexcept { return cost; }

double Trace::getAverageDuration() const noexcept { return averageDuration; }

std::ostream &palloc::operator<<(std::ostream &os, const Trace &trace) {
    os << "Trace(time=" << std::setw(5) << trace.getTimeStep() << ", requests=" << std::setw(5)
       << trace.getNumberOfRequests() << ", simulations=" << std::setw(5)
       << trace.getNumberOfOngoingSimulations() << ", parkingSpots=" << std::setw(5)
       << trace.getAvailableParkingSpots() << ", cost=" << std::setw(10) << trace.getCost()
       << ", averageDuration=" << std::setw(10) << trace.getAverageDuration() << ")";

    return os;
}

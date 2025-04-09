#include "trace.hpp"

using namespace palloc;

size_t Trace::getNumberOfOngoingSimulations() const noexcept { return _numberOfOngoingSimulations; }

size_t Trace::getDroppedRequests() const noexcept { return _droppedRequests; }

size_t Trace::getNumberOfRequests() const noexcept { return _numberOfRequests; }

uint64_t Trace::getAvailableParkingSpots() const noexcept { return _availableParkingSpots; }

uint64_t Trace::getTimeStep() const noexcept { return _timestep; }

double Trace::getCost() const noexcept { return _cost; }

double Trace::getAverageDuration() const noexcept { return _averageDuration; }

// Shallow copy, this is bad but not used enough to warrent implementing deep copy for all dependents.
Assignments Trace::getAssignments() const noexcept { return Assignments(_assignments); }

uint32_t Assignment::getRequestDuration() const noexcept { return _requestDuration; }

std::ostream &palloc::operator<<(std::ostream &os, const Trace &trace) {
    os << "Trace(time=" << std::setw(5) << trace.getTimeStep() << ", requests=" << std::setw(5)
       << trace.getNumberOfRequests() << ", simulations=" << std::setw(5)
       << trace.getNumberOfOngoingSimulations() << ", parkingSpots=" << std::setw(4)
       << trace.getAvailableParkingSpots() << ", cost=" << std::setw(8) << trace.getCost()
       << ", averageDuration=" << std::setw(10) << trace.getAverageDuration()
       << ", droppedRequests=" << std::setw(5) << trace.getDroppedRequests() << ")";

    return os;
}

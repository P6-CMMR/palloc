#include "trace.hpp"

using namespace palloc;

uint64_t Trace::getTimeStep() const noexcept { return timestep; }

size_t Trace::getNumberOfRequests() const noexcept { return numberOfRequests; }

size_t Trace::getNumberOfOngoingSimulations() const noexcept { return numberOfOngoingSimulations; }

uint64_t Trace::getAvailableParkingSpots() const noexcept { return availableParkingSpots; }

double Trace::getCost() const noexcept { return cost; }

double Trace::getAverageDuration() const noexcept { return averageDuration; }

size_t Trace::getDroppedRequests() const noexcept { return droppedRequests; }

std::ostream &palloc::operator<<(std::ostream &os, const Trace &trace) {
    os << "Trace(time=" << std::setw(5) << trace.getTimeStep() << ", requests=" << std::setw(5)
       << trace.getNumberOfRequests() << ", simulations=" << std::setw(5)
       << trace.getNumberOfOngoingSimulations() << ", parkingSpots=" << std::setw(4)
       << trace.getAvailableParkingSpots() << ", cost=" << std::setw(8) << trace.getCost()
       << ", averageDuration=" << std::setw(10) << trace.getAverageDuration() 
       << ", droppedRequests=" << std::setw(5) << trace.getDroppedRequests() << ")";

    return os;
}

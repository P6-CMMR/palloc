#include "trace.hpp"

using namespace palloc;

size_t Trace::getTimeStep() const noexcept { return timestep; }

size_t Trace::getNumberOfRequests() const noexcept { return numberOfRequests; }

size_t Trace::getNumberOfOngoingSimulations() const noexcept { return numberOfOngoingSimulations; }

size_t Trace::getAvailableParkingSpots() const noexcept { return availableParkingSpots; }

std::optional<size_t> Trace::getCost() const noexcept { return cost; }

std::optional<double> Trace::getAverageDuration() const noexcept { return averageDuration; }

void Trace::setAverageDuration(double averageDuration) noexcept {
    this->averageDuration = averageDuration;
}

void Trace::setCost(size_t cost) noexcept { this->cost = cost; }

std::ostream &palloc::operator<<(std::ostream &os, const Trace &trace) {
    os << "Trace(time=" << std::setw(5) << trace.getTimeStep() << ", requests=" << std::setw(5)
       << trace.getNumberOfRequests() << ", simulations=" << std::setw(5)
       << trace.getNumberOfOngoingSimulations() << ", parkingSpots=" << std::setw(5)
       << trace.getAvailableParkingSpots();
    if (trace.getCost().has_value()) {
        os << ", cost=" << std::setw(10) << trace.getCost().value();
    }
    if (trace.getAverageDuration().has_value()) {
        os << ", averageDuration=" << std::setw(10) << trace.getAverageDuration().value();
    }
    os << ")";
    return os;
}

#ifndef TRACE_HPP
#define TRACE_HPP

#include <cstdint>
#include <iomanip>
#include <list>
#include <ostream>

namespace palloc {
class Trace {
   public:
    explicit Trace(uint64_t timestep, size_t numberOfRequests, size_t numberOfOngoingSimulations,
                   size_t availableParkingSpots)
        : timestep(timestep),
          numberOfRequests(numberOfRequests),
          numberOfOngoingSimulations(numberOfOngoingSimulations),
          availableParkingSpots(availableParkingSpots) {}

    size_t getTimeStep() const noexcept;
    size_t getNumberOfRequests() const noexcept;
    size_t getNumberOfOngoingSimulations() const noexcept;
    size_t getAvailableParkingSpots() const noexcept;
    double getCost() const noexcept;
    double getAverageDuration() const noexcept;

    void setCost(double cost) noexcept;
    void setAverageDuration(double duration) noexcept;

   private:
    uint64_t timestep;
    size_t numberOfRequests;
    size_t numberOfOngoingSimulations;
    size_t availableParkingSpots;
    double cost = 0.0;
    double averageDuration = 0.0;
};

std::ostream &operator<<(std::ostream &os, const Trace &trace);

using Traces = std::list<Trace>;
}  // namespace palloc

#endif
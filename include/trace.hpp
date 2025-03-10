#ifndef TRACE_HPP
#define TRACE_HPP

#include <cstdint>
#include <iomanip>
#include <list>
#include <optional>
#include <ostream>

namespace palloc {
class Trace {
   public:
    explicit Trace(uint64_t timestep, size_t numberOfRequests, size_t numberOfOngoingSimulations,
                   size_t availableParkingSpots)
        : timestep(timestep),
          numberOfRequests(numberOfRequests),
          numberOfOngoingSimulations(numberOfOngoingSimulations),
          availableParkingSpots(availableParkingSpots),
          averageDuration(std::nullopt),
          cost(std::nullopt) {}

    size_t getTimeStep() const noexcept;
    size_t getNumberOfRequests() const noexcept;
    size_t getNumberOfOngoingSimulations() const noexcept;
    size_t getAvailableParkingSpots() const noexcept;
    std::optional<size_t> getCost() const noexcept;
    std::optional<double> getAverageDuration() const noexcept;

    void setCost(size_t cost) noexcept;
    void setAverageDuration(double duration) noexcept;

   private:
    uint64_t timestep;
    size_t numberOfRequests;
    size_t numberOfOngoingSimulations;
    size_t availableParkingSpots;
    std::optional<size_t> cost;
    std::optional<double> averageDuration;
};

std::ostream &operator<<(std::ostream &os, const Trace &trace);

using Traces = std::list<Trace>;
}  // namespace palloc

#endif
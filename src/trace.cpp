#include "trace.hpp"

using namespace palloc;

size_t Trace::getNumberOfOngoingSimulations() const noexcept { return _numberOfOngoingSimulations; }

size_t Trace::getDroppedRequests() const noexcept { return _droppedRequests; }

size_t Trace::getEarlyRequests() const noexcept { return _earlyRequests; }

size_t Trace::getNumberOfRequests() const noexcept { return _numberOfRequests; }

Uint Trace::getAvailableParkingSpots() const noexcept { return _availableParkingSpots; }

Uint Trace::getTimeStep() const noexcept { return _timestep; }

double Trace::getCost() const noexcept { return _averageCost; }

double Trace::getAverageDuration() const noexcept { return _averageDuration; }

Assignments Trace::getAssignments() const noexcept { return _assignments; }

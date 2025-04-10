#include "trace.hpp"

using namespace palloc;

size_t Trace::getNumberOfOngoingSimulations() const noexcept { return _numberOfOngoingSimulations; }

size_t Trace::getDroppedRequests() const noexcept { return _droppedRequests; }

size_t Trace::getEarlyRequests() const noexcept { return _earlyRequests; }

size_t Trace::getNumberOfRequests() const noexcept { return _numberOfRequests; }

uint32_t Trace::getAvailableParkingSpots() const noexcept { return _availableParkingSpots; }

uint32_t Trace::getTimeStep() const noexcept { return _timestep; }

double Trace::getCost() const noexcept { return _cost; }

double Trace::getAverageDuration() const noexcept { return _averageDuration; }

// Shallow copy, this is bad but not used enough to warrent implementing deep copy for all dependents.
Assignments Trace::getAssignments() const noexcept { return Assignments(_assignments); }

uint32_t Assignment::getRequestDuration() const noexcept { return _requestDuration; }


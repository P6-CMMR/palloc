#include "trace.hpp"

using namespace palloc;

size_t Trace::getNumberOfOngoingSimulations() const noexcept { return _numberOfOngoingSimulations; }

size_t Trace::getDroppedRequests() const noexcept { return _droppedRequests; }

size_t Trace::getNumberOfRequests() const noexcept { return _numberOfRequests; }

uint32_t Trace::getAvailableParkingSpots() const noexcept { return _availableParkingSpots; }

uint32_t Trace::getTimeStep() const noexcept { return _timestep; }

double Trace::getCost() const noexcept { return _cost; }

double Trace::getAverageDuration() const noexcept { return _averageDuration; }

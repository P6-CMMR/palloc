#include "request.hpp"

using namespace palloc;

uint32_t Request::getDropoffNode() const noexcept { return _dropoffNode; }

uint32_t Request::getRequestDuration() const noexcept { return _requestDuration; }

uint32_t Request::getArrival() const noexcept { return _tillArrival; }

uint32_t Request::getTimesDropped() const noexcept { return _timesDropped; }

void Request::decrementDuration() noexcept { --_requestDuration; }

void Request::decrementTillArrival() noexcept { --_tillArrival; }

void Request::incrementTimesDropped() noexcept { ++_timesDropped; }

bool Request::isDead() const noexcept { return _requestDuration == 0; }

bool Request::isEarly() const noexcept { return _tillArrival > 0; }
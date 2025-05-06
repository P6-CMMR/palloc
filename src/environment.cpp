#include "environment.hpp"

using namespace palloc;

const Environment::DurationMatrix &Environment::getDropoffToParking() const noexcept {
    return _dropoffToParking;
}

const Environment::DurationMatrix &Environment::getParkingToDropoff() const noexcept {
    return _parkingToDropoff;
}

UintVector &Environment::getAvailableParkingSpots() noexcept { return _availableParkingSpots; }

size_t Environment::getNumberOfDropoffs() const noexcept { return _dropoffToParking.size(); }

size_t Environment::getNumberOfParkings() const noexcept { return _parkingToDropoff.size(); }

void Environment::decrementParkingFor(size_t parkingIndex) {
    --_availableParkingSpots[parkingIndex];
    --_totalAvailableParkingSpots;
}

void Environment::incrementParkingFor(size_t parkingIndex) {
    ++_availableParkingSpots[parkingIndex];
    ++_totalAvailableParkingSpots;
}

Uint Environment::getTotalAvailableParkingSpots() { return _totalAvailableParkingSpots;}

const Environment::Coordinates &Environment::getDropoffCoordinates() const noexcept {
    return _dropoffCoords;
}

const Environment::Coordinates &Environment::getParkingCoordinates() const noexcept {
    return _parkingCoords;
}

const UintVector &Environment::getSmallestRoundTrips() const noexcept {
    return _smallestRoundTrips;
}

const DoubleVector &Environment::getParkingWeights() const noexcept { return _parkingWeights; }

void Environment::loadEnvironment(const std::filesystem::path &environmentPath) {
    if (!std::filesystem::exists(environmentPath)) {
        throw std::runtime_error("Environment file does not exist: " + environmentPath.string());
    }

    const auto error = glz::read_file_json<glz::opts{.error_on_unknown_keys = false}>(
        *this, environmentPath.string(), std::string{});
    if (error) {
        const auto errorStr = glz::format_error(error, std::string{});
        throw std::runtime_error("Failed to read environment file: " + environmentPath.string() +
                                 "\nwith error: " + errorStr);
    }
}
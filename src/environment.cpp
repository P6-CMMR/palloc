#include "environment.hpp"

using namespace palloc;

const Environment::DurationMatrix &Environment::getDropoffToParking() const noexcept {
    return dropoffToParking;
}

const Environment::DurationMatrix &Environment::getParkingToDropoff() const noexcept {
    return parkingToDropoff;
}

Environment::UintVector &Environment::getAvailableParkingSpots() noexcept {
    return availableParkingSpots;
}

const Environment::Coordinates &Environment::getDropoffCoordinates() const noexcept {
    return dropoffCoords;
}

const Environment::Coordinates &Environment::getParkingCoordinates() const noexcept {
    return parkingCoords;
}

size_t Environment::getNumberOfDropoffs() const noexcept { return dropoffToParking.size(); }

size_t Environment::getNumberOfParkings() const noexcept { return parkingToDropoff.size(); }

void Environment::loadEnvironment(const std::filesystem::path &environmentPath) {
    if (!std::filesystem::exists(environmentPath)) {
        throw std::runtime_error("Environment file does not exist: " + environmentPath.string());
    }

    const auto error = glz::read_file_json(*this, environmentPath.string(), std::string{});
    if (error) {
        throw std::runtime_error("Failed to read environment file: " + environmentPath.string());
    }
}
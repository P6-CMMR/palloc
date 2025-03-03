#include "environment.hpp"

using namespace palloc;

const Environment::DurationMatrix &Environment::getDropoffToParking() const noexcept {
    return dropoffToParking;
}

const Environment::DurationMatrix &Environment::getParkingToDropoff() const noexcept {
    return parkingToDropoff;
}

const Environment::IntVector &Environment::getParkingCapacities() const noexcept {
    return parkingCapacities;
}

void Environment::loadEnvironment(const std::filesystem::path &environmentPath) {
    if (!std::filesystem::exists(environmentPath)) {
        throw std::runtime_error("Environment file does not exist: " + environmentPath.string());
    }

    EnvironmentData data;
    const auto error = glz::read_file_json(data, environmentPath.string(), std::string{});
    if (error) {
        throw std::runtime_error("Failed to read environment file: " + environmentPath.string());
    }

    dropoffToParking = std::move(data.dropoffToParking);
    parkingToDropoff = std::move(data.parkingToDropoff);
    parkingCapacities = std::move(data.parkingCapacities);
}
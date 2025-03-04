#include "environment.hpp"

using namespace palloc;

const Environment::DurationMatrix &Environment::getDurationMatrix() const noexcept {
    return durationMatrix;
}

const Environment::IntVector &Environment::getParkingCapacities() const noexcept {
    return parkingCapacities;
}

const uint64_t Environment::getNumberOfParkings() const noexcept {
    return numberOfParkings;
}

const uint64_t Environment::getNumberOfDropoffs() const noexcept {
    return numberOfDropoffs;
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

    durationMatrix = std::move(data.durationMatrix);
    parkingCapacities = std::move(data.parkingCapacities);
    numberOfParkings = data.numberOfParkings;
    numberOfDropoffs = data.numberOfDropoffs;
}
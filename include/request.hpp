#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <vector>

#include "types.hpp"

namespace palloc {
class Request {
   public:
    explicit Request(Uint dropoffNode, Uint requestDuration, Uint tillArrival)
        : _dropoffNode(dropoffNode), _requestDuration(requestDuration), _tillArrival(tillArrival) {}

    Uint getDropoffNode() const noexcept;
    Uint getRequestDuration() const noexcept;
    Uint getTimesDropped() const noexcept;
    Uint getArrival() const noexcept;

    void decrementDuration() noexcept;
    void decrementTillArrival() noexcept;
    void incrementTimesDropped() noexcept;

    bool isDead() const noexcept;
    bool isEarly() const noexcept;

   private:
    Uint _dropoffNode;
    Uint _requestDuration;
    Uint _timesDropped = 0;
    Uint _tillArrival;
};

using Requests = std::vector<Request>;
}  // namespace palloc

#endif
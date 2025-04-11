#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <cstdint>
#include <vector>

namespace palloc {
    class Request {
        public:
         explicit Request(uint32_t dropoffNode, uint32_t requestDuration, uint32_t tillArrival)
             : _dropoffNode(dropoffNode), _requestDuration(requestDuration), _tillArrival(tillArrival) {}
     
         uint32_t getDropoffNode() const noexcept;
         uint32_t getRequestDuration() const noexcept;
         uint32_t getTimesDropped() const noexcept;
         uint32_t getArrival() const noexcept;
     
         void decrementDuration() noexcept;
         void decrementTillArrival() noexcept;
         void incrementTimesDropped() noexcept;
     
         bool isDead() const noexcept;
         bool isEarly() const noexcept;
     
        private:
         uint32_t _dropoffNode;
         uint32_t _requestDuration;
         uint32_t _timesDropped = 0;
         uint32_t _tillArrival;
     };
     
     using Requests = std::vector<Request>;
}

#endif
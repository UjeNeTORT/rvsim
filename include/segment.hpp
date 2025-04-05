#ifndef SEGMENT_HPP
#define SEGMENT_HPP

#include <cstdint>

#include "encoding.hpp"

namespace rv32i_sim {

constexpr std::size_t DEFAULT_SEG_SIZE = 1 << 16;
constexpr std::size_t DEFAULT_ALIGN = 4; // 4 bytes

constexpr std::size_t MAX_STACK_SIZE = 1 << 20;

constexpr uint8_t RIGHTS_R = 1;
constexpr uint8_t RIGHTS_W = 2;
constexpr uint8_t RIGHTS_X = 4;
constexpr uint8_t DEFAULT_RIGHTS = RIGHTS_R; // READ

/// @brief describes a segment of memory
/// @brief but does not store any data
class Segment {
  addr_t vaddr_; ///< begin address of a segment
  addr_t size_; ///< size in the direction of increasing addresses
  uint8_t rights_; ///< RWX
  uint8_t align_; ///< alignment

public:
  Segment() {}
  Segment(addr_t vaddr,
          addr_t size = DEFAULT_SEG_SIZE,
          uint8_t rights = DEFAULT_RIGHTS,
          uint8_t align = DEFAULT_ALIGN);

  addr_t getVaddr() const;
  addr_t getSize() const;
  uint8_t getRights() const;
  uint8_t getAlign() const;

  bool checkRights(uint8_t rights) const;

  static Segment createSegment(addr_t vaddr,
                               addr_t size = DEFAULT_SEG_SIZE,
                               uint8_t rights = DEFAULT_RIGHTS,
                               uint8_t align = DEFAULT_ALIGN);

  virtual ~Segment() = default;
};

} // rv32i_sim

#endif // SEGMENT_HPP

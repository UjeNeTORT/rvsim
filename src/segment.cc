#include "segment.hpp"

namespace rv32i_sim {

Segment::Segment(addr_t vaddr, addr_t size, uint8_t rights, uint8_t align) :
          vaddr_(vaddr), size_(size), rights_(rights), align_(align) {

  // this is done to fight rights violation in example test/elf/plus.elf
  // elfio returns rights of section .data to be WX, while they are actually RW
  // todo figure out the reason and fix it, or report elfio bug
  if (rights & RIGHTS_W) rights_ |= RIGHTS_R;
}

addr_t Segment::getVaddr() const {
  return vaddr_;
}

addr_t Segment::getSize() const {
  return size_;
}

uint8_t Segment::getRights() const {
  return rights_;
}

uint8_t Segment::getAlign() const {
  return align_;
}

bool Segment::checkRights(uint8_t rights) const {
  return rights_ & rights;
}

Segment Segment::createSegment(addr_t vaddr, addr_t size,
                               uint8_t rights, uint8_t align) {

  return Segment(vaddr, size, rights, align);
}

} // rv32i_sim

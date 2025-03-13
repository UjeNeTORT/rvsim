#ifndef MEMORY_HPP
#define MEMORY_HPP

#include "encoding.hpp"


namespace rv32i_sim {

const unsigned BITS_BYTE = 8; // n bits in byte
constexpr std::size_t ADDR_SPACE_CAPACITY = 1 << 16; // 64K - less than all
                                                     // addr space

/** provides memory model for the simulator
 * model is abstract and represents continuous virtual memory
 *
 * endianness: small
*/
class MemoryModel {
  std::array<byte_t, ADDR_SPACE_CAPACITY> mem_;

public:
  explicit MemoryModel() {}

  // todo endianness

  byte_t readByte(addr_t addr)  {
    return mem_[addr];
  }

  half_t readHalf(addr_t addr)  {
    half_t res = 0;
    for (int i = sizeof(half_t); i != 0; --i) {
      res <<= sizeof(byte_t) * 8;
      res |= (half_t) mem_[addr];
      addr++;
    }

    return res;
  }

  word_t readWord(addr_t addr) {
    word_t res = 0;
    for (int i = sizeof(word_t); i != 0; --i) {
      res <<= sizeof(byte_t) * BITS_BYTE;
      res |= (word_t) mem_[addr];
      addr++;
    }

    return res;
  }

#ifdef RVBITS64
  dword_t readDWord(addr_t addr) {
    dword_t res = 0;
    for (int i = sizeof(dword_t); i != 0; --i) {
      res << sizeof(byte_t);
      res |= (dword_t) mem_[addr];
      addr++;
    }

    return res;
  }
#endif // RVBITS64

  // todo check permissions

  void writeByte(addr_t addr, byte_t val) {
    mem_[addr] = val;
  }

  void writeHalf(addr_t addr, half_t val) {
    for (int i = 0; i != sizeof(half_t); ++i) {
      byte_t curr = val & 0xFF;
      mem_[addr++] = curr;
      val >>= BITS_BYTE; // next byte
    }
  }

  void writeWord(addr_t addr, word_t val) {
    for (int i = 0; i != sizeof(word_t); ++i) {
      byte_t curr = val & 0xFF;
      mem_[addr++] = curr;
      val >>= BITS_BYTE; // next byte
    }
  }

#ifdef RVBITS64
  void writeDWord(addr_t addr, dword_t val) {
    for (int i = 0; i != sizeof(word_t); ++i) {
      byte_t curr = val & 0xFF;
      mem_[addr++] = curr;
      val >>= BITS_BYTE; // next byte
    }
  }
#endif // RVBITS64

};

} // rv32i_sim

#endif // MEMORY_HPP

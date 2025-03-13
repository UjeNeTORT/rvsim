#ifndef MEMORY_HPP
#define MEMORY_HPP

#include "encoding.hpp"


namespace rv32i_sim {

const unsigned BITS_BYTE = 8; // n bits in byte
constexpr std::size_t ADDR_SPACE_CAPACITY = 1 << 16; // 64K - less than all
                                                     // addr space

enum class Endianness { SMALL, BIG, }; // todo support big endian

/** provides memory model for the simulator
 * model is abstract and represents continuous virtual memory
 *
 * endianness: small (default)
*/
class MemoryModel {
  std::array<byte_t, ADDR_SPACE_CAPACITY> mem_ = {};
  Endianness endian_ = Endianness::SMALL;

public:
  MemoryModel(Endianness endian = Endianness::SMALL) {}

  explicit
  MemoryModel(std::array<byte_t, ADDR_SPACE_CAPACITY> mem) : mem_(mem) {}

  inline byte_t readByte(addr_t addr) {
    return mem_[addr];
  }

  inline half_t readHalf(addr_t addr) {
    half_t res = 0;
    for (int i = sizeof(half_t) - 1; i >= 0; --i) {
      res <<= sizeof(byte_t) * BITS_BYTE;
      res |= half_t(mem_[addr + i]);
    }

    return res;
  }

  inline word_t readWord(addr_t addr) {
    word_t res = 0;
    for (int i = sizeof(word_t) - 1; i >= 0; --i) {
      res <<= sizeof(byte_t) * BITS_BYTE;
      res |= word_t(mem_[addr + i]);
    }

    return res;
  }

#ifdef RVBITS64
  inline dword_t readDWord(addr_t addr) {
    dword_t res = 0;
    for (int i = sizeof(dword_t) - 1; i >= 0; --i) {
      res << sizeof(byte_t) * BITS_BYTE;
      res |= dword_t(mem_[addr + i]);
    }

    return res;
  }

#endif // RVBITS64

  // todo check permissions
  inline void writeByte(addr_t addr, byte_t val) {
    mem_[addr] = val;
  }

  inline void writeHalf(addr_t addr, half_t val) {
    for (int i = 0; i != sizeof(half_t); ++i) {
      byte_t curr = val & 0xFF;
      mem_[addr++] = curr;
      val >>= BITS_BYTE; // next byte
    }
  }

  inline void writeWord(addr_t addr, word_t val) {
    for (int i = 0; i != sizeof(word_t); ++i) {
      byte_t curr = val & 0xFF;
      mem_[addr++] = curr;
      val >>= BITS_BYTE; // next byte
    }
  }

#ifdef RVBITS64
  inline void writeDWord(addr_t addr, dword_t val) {
    for (int i = 0; i != sizeof(word_t); ++i) {
      byte_t curr = val & 0xFF;
      mem_[addr++] = curr;
      val >>= BITS_BYTE; // next byte
    }
  }
#endif // RVBITS64

  std::ostream& dump(std::ostream& out, addr_t start_addr, addr_t end_addr) {
    if (start_addr > end_addr) {
      out << "| <err-start-greater-than-end> \n";
      return out;
    }

    for (addr_t i = start_addr; i != end_addr; i += 2 * sizeof(word_t)) {
      out << "| " << std::hex
                  << +readByte(i+0) << ' ' << +readByte(i+1) << ' '
                  << +readByte(i+2) << ' ' << +readByte(i+3) << " | "
                  << +readByte(i+4) << ' ' << +readByte(i+5) << ' '
                  << +readByte(i+6) << ' ' << +readByte(i+7) << " |\n";
    }

    return out;
  }

  std::ostream& dump(std::ostream& out) {
    dump(out, 0, ADDR_SPACE_CAPACITY);
    return out;
  }
};

std::ostream& operator<<(std::ostream& out, MemoryModel& memory) {
  memory.dump(out);
  return out;
}

} // rv32i_sim

#endif // MEMORY_HPP

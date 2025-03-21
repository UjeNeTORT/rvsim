#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <fstream>
#include <filesystem>
#include <string>

#include "encoding.hpp"

namespace rv32i_sim {

constexpr std::size_t ADDR_SPACE_CAPACITY = 1 << 16; // 64K - addr space
enum class Endianness { LITTLE, BIG, }; // todo support big endian

const std::string RV32I_MEMORY_STATE_SIGNATURE = "RV32I_MEM_STATE";

/** provides memory model for the simulator
 * model is abstract and represents continuous virtual memory
 *
 * endianness: little (default)
*/
class MemoryModel {
  std::array<byte_t, ADDR_SPACE_CAPACITY> mem_ = {};
  Endianness endian_ = Endianness::LITTLE;

public:
  MemoryModel(Endianness endian = Endianness::LITTLE) : endian_(endian) {}

  MemoryModel(std::array<byte_t, ADDR_SPACE_CAPACITY> mem) : mem_(mem) {}

  MemoryModel(std::ifstream& mem_file) {
    if (!mem_file) {
      std::cerr << "ERROR: wrong memory file\n";
    }

    std::string signature(RV32I_MEMORY_STATE_SIGNATURE.size(), ' ');
    mem_file.read(signature.data(), RV32I_MEMORY_STATE_SIGNATURE.size() + 1);

    if (signature != RV32I_MEMORY_STATE_SIGNATURE) {
      std::cerr << "ERROR: mem state file signature mismatch:\n"
                << "      <" << signature << "> vs <" << RV32I_MEMORY_STATE_SIGNATURE <<">\n";
      return;
    }

    mem_file.read(std::bit_cast<char *>(mem_.data()), ADDR_SPACE_CAPACITY);
  }

  bool operator==(const MemoryModel& other) const {
    return endian_ == other.endian_ && mem_ == other.mem_;
  }

  byte_t readByte(addr_t addr) {
    return mem_[addr];
  }

  half_t readHalf(addr_t addr) {
    half_t res = 0;
    for (int i = sizeof(half_t) - 1; i >= 0; --i) {
      res <<= sizeof(byte_t) * BITS_BYTE;
      res |= half_t(mem_[addr + i]);
    }

    return res;
  }

  word_t readWord(addr_t addr) {
    word_t res = 0;
    for (int i = sizeof(word_t) - 1; i >= 0; --i) {
      res <<= sizeof(byte_t) * BITS_BYTE;
      res |= word_t(mem_[addr + i]);
    }

    return res;
  }

#ifdef RVBITS64
  dword_t readDWord(addr_t addr) {
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

    // todo i know this is bad, dont want to generalize this at the moment
    for (addr_t i = start_addr; i != end_addr; i += 4 * sizeof(word_t)) {
      out << "| " << std::hex << std::uppercase
                  << +readByte(i+0) << ' ' << +readByte(i+1) << ' '
                  << +readByte(i+2) << ' ' << +readByte(i+3) << " | "
                  << +readByte(i+4) << ' ' << +readByte(i+5) << ' '
                  << +readByte(i+6) << ' ' << +readByte(i+7) << " | "
                  << +readByte(i+8) << ' ' << +readByte(i+9) << ' '
                  << +readByte(i+10) << ' ' << +readByte(i+11) << " | "
                  << +readByte(i+12) << ' ' << +readByte(i+13) << ' '
                  << +readByte(i+14) << ' ' << +readByte(i+15) << " |\n"
                  << std::nouppercase;
    }

    return out;
  }

  std::ostream& print(std::ostream& out) {
    dump(out, 0, ADDR_SPACE_CAPACITY);
    return out;
  }

  void binaryDump(std::ofstream& fout) {
    fout.write(RV32I_MEMORY_STATE_SIGNATURE.c_str(),
               RV32I_MEMORY_STATE_SIGNATURE.size() + 1);

    fout.write(reinterpret_cast<char *>(mem_.data()), ADDR_SPACE_CAPACITY);
  }
};

std::ostream& operator<<(std::ostream& out, MemoryModel& memory) {
  memory.print(out);
  return out;
}

} // rv32i_sim

#endif // MEMORY_HPP

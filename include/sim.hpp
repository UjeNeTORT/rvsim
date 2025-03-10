#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include <array>
#include <cstdint>
#include <iostream>
#include <vector>

namespace rv32i_sim {

#ifdef RVBITS64
using addr_t = uint64_t;
#else // RVBITS64
using addr_t = uint64_t;
#endif // RVBITS64

using byte_t = uint8_t;
using half_t = uint16_t;
using word_t = uint32_t;

#ifdef RVBITS64
using dword_t = uint64_t;
#endif // RVBITS64

enum Signal {
  SIGILL = 0, // illegal instruction
};

const unsigned BITS_BYTE = 8; // n bits in byte

constexpr std::size_t N_REGS = 32; // GP registers 4 register file
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

class RegisterFile {
  std::array<addr_t, N_REGS> regs_;

public:
  RegisterFile() {}
  explicit
  RegisterFile(std::array<addr_t, N_REGS> init_regs_state) :
                                            regs_(init_regs_state) {}

  void set(unsigned n_reg, addr_t val) {
    if (n_reg < N_REGS) regs_[n_reg] = val;
    else throw SIGILL; //? should i
  }

  addr_t get(unsigned n_reg) {
    if (n_reg < N_REGS) return regs_[n_reg];
    else throw SIGILL; //? should i
  }
};

class RVModel{
  MemoryModel mem_;
  RegisterFile regs_;
  addr_t pc_;

public:
  explicit RVModel() {}

  void init() {}

  void setPC(addr_t pc_new) {
    pc_ = pc_new;
  }

  void execute() {

  }
};

} // namespace rv32i_sim

#endif // SIMULATOR_HPP

#ifndef REGISTERS_HPP
#define REGISTERS_HPP

#include "encoding.hpp"

namespace rv32i_sim {

constexpr std::size_t N_REGS = 32; // number of registers

enum Registers {
  X0 = 0,
  X1 = 1,
  X2 = 2,
  X3 = 3,
  X4 = 4,
  X5 = 5,
  X6 = 6,
  X7 = 7,
  X8 = 8,
  X9 = 9,
  X10 = 10,
  X11 = 11,
  X12 = 12,
  X13 = 13,
  X14 = 14,
  X15 = 15,
  X16 = 16,
  X17 = 17,
  X18 = 18,
  X19 = 19,
  X20 = 20,
  X21 = 21,
  X22 = 22,
  X23 = 23,
  X24 = 24,
  X25 = 25,
  X26 = 26,
  X27 = 27,
  X28 = 28,
  X29 = 29,
  X30 = 30,
  X31 = 31,
};

class RegisterFile {
  std::array<addr_t, N_REGS> regs_;

public:
  RegisterFile() {}

  explicit
  RegisterFile(std::array<addr_t, N_REGS> init_regs_state) :
                                            regs_(init_regs_state) {}

  // todo errors
  void set(unsigned n_reg, addr_t val) {
    regs_[n_reg] = val;
  }

  // todo errors
  addr_t get(unsigned n_reg) {
    return regs_[n_reg];
  }
};

} // rv32i_sim

#endif // REGISTERS_HPP

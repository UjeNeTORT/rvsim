#ifndef REGISTERS_HPP
#define REGISTERS_HPP

#include <cstdint>

namespace rv32i_sim {

enum class Register : uint32_t {
  X0  =  0, ZERO = 0,
  X1  =  1, RA  = 1,
  X2  =  2, SP  = 2,
  X3  =  3, GP  = 3,
  X4  =  4, TP  = 4,
  X5  =  5, T0  = 5,
  X6  =  6, T1  = 6,
  X7  =  7, T2  = 7,
  X8  =  8, S0  = 8, FP = 8,
  X9  =  9, S1  = 9,
  X10 = 10, A0  = 10,
  X11 = 11, A1  = 11,
  X12 = 12, A2  = 12,
  X13 = 13, A3  = 13,
  X14 = 14, A4  = 14,
  X15 = 15, A5  = 15,
  X16 = 16, A6  = 16,
  X17 = 17, A7  = 17,
  X18 = 18, S2  = 18,
  X19 = 19, S3  = 19,
  X20 = 20, S4  = 20,
  X21 = 21, S5  = 21,
  X22 = 22, S6  = 22,
  X23 = 23, S7  = 23,
  X24 = 24, S8  = 24,
  X25 = 25, S9  = 25,
  X26 = 26, S10 = 26,
  X27 = 27, S11 = 27,
  X28 = 28, T3  = 28,
  X29 = 29, T4  = 29,
  X30 = 30, T5  = 30,
  X31 = 31, T6  = 31,
  INVALID = 0xFF,
};

constexpr std::size_t N_REGS = 32; // number of registers

} // namespace rv32i_sim

#endif // REGISTERS_HPP

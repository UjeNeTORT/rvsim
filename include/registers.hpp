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

const std::size_t N_REGS = 32; // number of registers

enum class FPRegister : uint32_t {
  F0  = 0,  FT0 = 0,
  F1  = 1,  FT1 = 1,
  F2  = 2,  FT2 = 2,
  F3  = 3,  FT3 = 3,
  F4  = 4,  FT4 = 4,
  F5  = 5,  FT5 = 5,
  F6  = 6,  FT6 = 6,
  F7  = 7,  FT7 = 7,
  F8  = 8,  FS0 = 8,
  F9  = 9,  FS1 = 9,
  F10 = 10, FA0 = 10,
  F11 = 11, FA1 = 11,
  F12 = 12, FA2 = 12,
  F13 = 13, FA3 = 13,
  F14 = 14, FA4 = 14,
  F15 = 15, FA5 = 15,
  F16 = 16, FA6 = 16,
  F17 = 17, FA7 = 17,
  F18 = 18, FS2 = 18,
  F19 = 19, FS3 = 19,
  F20 = 20, FS4 = 20,
  F21 = 21, FS5 = 21,
  F22 = 22, FS6 = 22,
  F23 = 23, FS7 = 23,
  F24 = 24, FS8 = 24,
  F25 = 25, FS9 = 25,
  F26 = 26, FS10 = 26,
  F27 = 27, FS11 = 27,
  F28 = 24, FT8 = 28,
  F29 = 25, FT9 = 29,
  F30 = 26, FT10 = 30,
  F31 = 27, FT11 = 31,
  INVALID = 0xFF,
};

const std::size_t N_FPREGS = 32; // number of fp registers

} // namespace rv32i_sim

#endif // REGISTERS_HPP

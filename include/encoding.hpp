#ifndef ENCODING_HPP
#define ENCODING_HPP

#include <cstdint>
#include <bitset>

#include "registers.hpp"

namespace rv32i_sim {

#ifdef RVBITS64
using addr_t = uint64_t;
#else // RVBITS64
using addr_t = uint32_t;
#endif // RVBITS64

using byte_t = uint8_t;
using half_t = uint16_t;
using word_t = uint32_t;
using sbyte_t = int8_t;
using shalf_t = int16_t;
using sword_t = int32_t;

#ifdef RVBITS64
using dword_t = uint64_t;
using sdword_t = int64_t;
#endif // RVBITS64

constexpr unsigned BITS_BYTE = 8; // n bits in byte
constexpr unsigned IALIGN = 4; // bytes, insn address alignment

enum class RVInsnType : uint8_t {
  UNDEF_TYPE_INSN = 0,
  R_TYPE_INSN = 1,
  I_TYPE_INSN = 2,
  S_TYPE_INSN = 3,
  B_TYPE_INSN = 4,
  U_TYPE_INSN = 5,
  J_TYPE_INSN = 6,
};

constexpr uint32_t MASK_31_25 = 0xFE000000;
constexpr uint32_t MASK_24_20 = 0x01F00000;
constexpr uint32_t MASK_19_15 = 0x000F8000;
constexpr uint32_t MASK_14_12 = 0x00007000;
constexpr uint32_t MASK_11_7  = 0x00000F80;
constexpr uint32_t MASK_6_0   = 0x0000007F;
constexpr uint32_t MASK_4_0   = 0x0000001F;

constexpr uint32_t MASK_31_20 = 0xFFF00000;
constexpr uint32_t MASK_31_12 = 0xFFFFF000;

constexpr uint32_t DEFAULT_OPCODE_MASK = MASK_6_0;
constexpr uint32_t DEFAULT_RD_MASK = MASK_11_7;
constexpr uint32_t DEFAULT_FUNC3_MASK = MASK_14_12;
constexpr uint32_t DEFAULT_RS1_MASK = MASK_19_15;
constexpr uint32_t DEFAULT_RS2_MASK = MASK_24_20;
constexpr uint32_t DEFAULT_FUNC7_MASK = MASK_31_25;

enum class RV32i_ISA : addr_t {

  // R-Type
  ADD = 0x00000033,
  SUB = 0x40000033,
  SLL = 0x00001033,
  SLT = 0x00002033,
  SLTU = 0x00003033,
  XOR = 0x00004033,
  SRL = 0x00005033,
  SRA = 0x40005033,
  OR = 0x00006033,
  AND = 0x00007033,

  // I-Type
  JALR = 0x00000067,
  LB = 0x00000003,
  LH = 0x00001003,
  LW = 0x00002003,
  LBU = 0x00004003,
  LHU = 0x00005003,
  ADDI = 0x00000013,
  SLTI = 0x00002013,
  SLTIU = 0x00003013,
  XORI = 0x00004013,
  ORI = 0x00006013,
  ANDI = 0x00007013,
  SLLI = 0x00001013,
  SRLI = 0x00005013,
  SRAI = 0x40005013, // poor creature... how should i decode you?

  // S-Type
  SB = 0x00000023,
  SH = 0x00001023,
  SW = 0x00002023,

  // B-Type
  BEQ = 0x00000063,
  BNE = 0x00001063,
  BLT = 0x00004063,
  BLTU = 0x00006063,
  BGE = 0x00005063,
  BGEU = 0x00007063,

  // U-Type
  LUI = 0x00000037,
  AUIPC = 0x00000017,

  // J-Type
  JAL = 0x0000006f,
};

constexpr uint8_t RV_R_TYPE_OPCODE = 0b011'0011;

constexpr uint8_t RV_I_TYPE_OPCODE     = 0b001'0011;
constexpr uint8_t RV_ILOAD_TYPE_OPCODE = 0b000'0011;
constexpr uint8_t RV_IJALR_TYPE_OPCODE = 0b110'0111;

constexpr uint8_t RV_S_TYPE_OPCODE  = 0b010'0011;

constexpr uint8_t RV_B_TYPE_OPCODE = 0b110'0011;

constexpr uint8_t RV_U1_TYPE_OPCODE = 0b011'0111;
constexpr uint8_t RV_U2_TYPE_OPCODE = 0b001'0111;

constexpr uint8_t RV_JAL_OPCODE = 0b110'1111;

} // rv32i_sim

#endif // ENCODING_HPP

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


enum class RVInsnType : uint8_t {
  UNDEF_TYPE_INSN = 0,
  R_TYPE_INSN = 1,
  I_TYPE_INSN = 2,
  S_TYPE_INSN = 3,
  U_TYPE_INSN = 4,
  B_TYPE_INSN = 5,
  J_TYPE_INSN = 6,
};

constexpr uint32_t MASK_31_25 = 0xFF000000;
constexpr uint32_t MASK_24_20 = 0x01F00000;
constexpr uint32_t MASK_19_15 = 0x000F8000;
constexpr uint32_t MASK_14_12 = 0x00007000;
constexpr uint32_t MASK_11_7  = 0x00000F80;
constexpr uint32_t MASK_6_0   = 0x0000007F;

constexpr uint32_t MASK_31_20 = 0xFFF00000;
constexpr uint32_t MASK_31_12 = 0xFFFFF000;

constexpr uint32_t DEFAULT_OPCODE_MASK = MASK_6_0;
constexpr uint32_t DEFAULT_RD_MASK = MASK_11_7;
constexpr uint32_t DEFAULT_FUNC3_MASK = MASK_14_12;;
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

  // U-Type
};

constexpr uint8_t RV_R_TYPE_OPCODE = 0b011'0011;

constexpr uint8_t RV_I_TYPE_OPCODE     = 0b001'0011;
constexpr uint8_t RV_ILOAD_TYPE_OPCODE = 0b000'0011; // todo
constexpr uint8_t RV_IJALR_TYPE_OPCODE = 0b110'0111;

constexpr uint8_t RV_S_TYPE_OPCODE  = 0b010'0011; // todo
constexpr uint8_t RV_SB_TYPE_OPCODE = 0b110'0011; // todo

constexpr uint8_t RV_U1_TYPE_OPCODE = 0b011'0111; // todo
constexpr uint8_t RV_U2_TYPE_OPCODE = 0b001'0111; // todo

class RVInsn {
  addr_t code_; // instr itself
  addr_t code_opless_; // instr encoding without operands

  uint32_t imm31_12_ = 0; // 20 bits
  uint16_t imm11_0_ = 0; // 12 bits

  RVInsnType type_ = RVInsnType::UNDEF_TYPE_INSN;
  uint8_t opcode_ = 0; // 7 bits

  uint8_t rd_ = 0;    // 5 bits
  uint8_t func3_ = 0; // 3 bits
  uint8_t rs1_ = 0;   // 5 bits
  uint8_t rs2_ = 0;   // 5 bits
  uint8_t func7_ = 0; // 7 bits

  uint8_t imm11_5_ = 0; // 7 bits
  uint8_t imm4_0_ = 0; // 5 bits

public:
  RVInsn(RVInsnType type = RVInsnType::UNDEF_TYPE_INSN) : type_(type) {}

  // opcode = code & DEFAULT_OPCODE_MASK
  explicit RVInsn(addr_t code) : code_(code), opcode_(code & DEFAULT_OPCODE_MASK)
  {
    rd_       = (code >> 7)  & ((1 << 5)  - 1);
    func3_    = (code >> 12) & ((1 << 3)  - 1);
    rs1_      = (code >> 15) & ((1 << 5)  - 1);
    rs2_      = (code >> 20) & ((1 << 5)  - 1);
    func7_    = (code >> 25) & ((1 << 7)  - 1);
    imm11_0_  = (code >> 20) & ((1 << 12) - 1);
    imm11_5_  = (code >> 25) & ((1 << 7)  - 1);
    imm4_0_   = (code >> 7)  & ((1 << 5)  - 1);
    imm31_12_ = (code >> 12) & ((1 << 20) - 1);

    switch (opcode_) {
    case RV_R_TYPE_OPCODE:
      type_ = RVInsnType::R_TYPE_INSN;
      code_opless_ = code & (addr_t(func7_) << 25 | addr_t(func3_) << 12 | opcode_);
      break;

    case RV_I_TYPE_OPCODE:
    case RV_ILOAD_TYPE_OPCODE:
    case RV_IJALR_TYPE_OPCODE:
      type_ = RVInsnType::I_TYPE_INSN;
      code_opless_ = code & (addr_t(func3_) << 12 | opcode_);
      break;
    case RV_S_TYPE_OPCODE:
    case RV_SB_TYPE_OPCODE:
      type_ = RVInsnType::S_TYPE_INSN;
      // todo decode opless
      break;
    case RV_U1_TYPE_OPCODE:
    case RV_U2_TYPE_OPCODE:
      type_ = RVInsnType::U_TYPE_INSN;
      // todo decode opless
      break;
    default:
      type_ = RVInsnType::UNDEF_TYPE_INSN;
      // todo decode to hlt?
    }
  }

  uint8_t opcode() const { return opcode_; }
  addr_t code() const { return code_; }
  addr_t code_opless() const { return code_opless_; }
  RVInsnType type() const { return type_; }

  Register rd()  const { return static_cast<Register>(rd_); }
  Register rs1() const { return static_cast<Register>(rs1_); }
  Register rs2() const { return static_cast<Register>(rs2_); }
  uint8_t func3() const { return func3_; }
  uint8_t func7() const { return func7_; }

  uint16_t imm11_0()  const { return imm11_0_; }
  uint8_t  imm11_5()  const { return imm11_5_; }
  uint8_t  imm4_0()   const { return imm4_0_; };
  uint32_t imm31_12() const { return imm31_12_; }

  // todo implement other types
  void print(std::ostream &out) const {
    switch (type_)
    {
    case RVInsnType::R_TYPE_INSN:
      out << std::bitset<7>{func7_} << "'"
          << std::bitset<5>{rs2_} << "'"
          << std::bitset<5>{rs1_} << "'"
          << std::bitset<3>{func3_} << "'"
          << std::bitset<5>{rd_} << "'"
          << std::bitset<7>{opcode_} << " (R)";
      break;
    case RVInsnType::I_TYPE_INSN:
      out << std::bitset<12>{imm11_0_} << "'"
          << std::bitset<5>{rs1_} << "'"
          << std::bitset<3>{func3_} << "'"
          << std::bitset<5>{rd_} << "'"
          << std::bitset<7>{opcode_} << " (I)";
      break;
    case RVInsnType::S_TYPE_INSN:
    case RVInsnType::U_TYPE_INSN:
    case RVInsnType::UNDEF_TYPE_INSN:
      out << std::bitset<32>{code_} << " (U)";
      break;
    default:
      out << std::bitset<32>{code_} << " <error-insn-type>";
    }
  };

  ~RVInsn() = default;
};

std::ostream& operator<<(std::ostream& out, const RVInsn &insn) {
  insn.print(out);
  return out;
}

} // rv32i_sim

#endif // ENCODING_HPP

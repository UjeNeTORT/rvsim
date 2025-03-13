#ifndef ENCODING_HPP
#define ENCODING_HPP

#include <cstdint>
#include <bitset>

namespace rv32i_sim {

#ifdef RVBITS64
using addr_t = uint64_t;
#else // RVBITS64
using addr_t = uint32_t;
#endif // RVBITS64

using byte_t = uint8_t;
using half_t = uint16_t;
using word_t = uint32_t;

#ifdef RVBITS64
using dword_t = uint64_t;
#endif // RVBITS64

constexpr uint8_t RV_OPCODE_MASK = 0b0111'1111;

enum RVInsnType {
  R_TYPE_INSN = 0,
  I_TYPE_INSN = 1,
  S_TYPE_INSN = 2,
  U_TYPE_INSN = 3,
  B_TYPE_INSN = 4,
  J_TYPE_INSN = 5,
  UNDEF_TYPE_INSN = 6,
};

namespace RV32i_ISA {

// R-Type
constexpr addr_t ADD = 0x00000033;
constexpr addr_t SUB = 0x40000033;
constexpr addr_t SLL = 0x00001033;
constexpr addr_t SLT = 0x00002033;
constexpr addr_t SLTU = 0x00003033;
constexpr addr_t XOR = 0x00004033;
constexpr addr_t SRL = 0x00005033;
constexpr addr_t SRA = 0x40005033;
constexpr addr_t OR = 0x00006033;
constexpr addr_t AND = 0x00007033;

// I-Type
// S-Type
// U-Type
}

constexpr uint8_t RV_R_TYPE_OPCODE = 0b011'0011;

constexpr uint8_t RV_I_TYPE_OPCODE     = 0b001'0011;
constexpr uint8_t RV_ILOAD_TYPE_OPCODE = 0b000'0011;
constexpr uint8_t RV_IJALR_TYPE_OPCODE = 0b110'0111;

constexpr uint8_t RV_S_TYPE_OPCODE  = 0b010'0011;
constexpr uint8_t RV_SB_TYPE_OPCODE = 0b110'0011;

constexpr uint8_t RV_U1_TYPE_OPCODE = 0b011'0111;
constexpr uint8_t RV_U2_TYPE_OPCODE = 0b001'0111;

class RVInsn {
  addr_t code_; // instr itself
  addr_t code_opless_; // instr encoding without operands

  uint32_t imm31_12_ = 0; // 20 bits
  uint16_t imm11_0_ = 0; // 12 bits

  RVInsnType type_ = UNDEF_TYPE_INSN;
  uint8_t opcode_ = 0; // 7 bits

  uint8_t rd_ = 0;    // 5 bits
  uint8_t func3_ = 0; // 3 bits
  uint8_t rs1_ = 0;   // 5 bits
  uint8_t rs2_ = 0;   // 5 bits
  uint8_t func7_ = 0; // 7 bits

  uint8_t imm11_5_ = 0; // 7 bits
  uint8_t imm4_0_ = 0; // 5 bits

public:
  RVInsn(RVInsnType type = UNDEF_TYPE_INSN) : type_(type) {}

  // opcode = code & RV_OPCODE_MASK
  explicit RVInsn(addr_t code) : code_(code), opcode_(code & RV_OPCODE_MASK)
  {
    rd_ = (code >> 7) & ((1 << 5) - 1);
    func3_ = (code >> 12) & ((1 << 3) - 1);
    rs1_ = (code >> 15) & ((1 << 5) - 1);
    rs2_ = (code >> 20) & ((1 << 5) - 1);
    func7_ = (code >> 25) & ((1 << 7) - 1);
    imm11_0_ = (code >> 20) & ((1 << 12) - 1);
    imm11_5_ = (code >> 25) & ((1 << 7) - 1);
    imm11_5_ = (code >> 7) & ((1 << 5) - 1);
    imm31_12_ = (code >> 12) & ((1 << 20) - 1);

    switch (opcode_) {
    case RV_R_TYPE_OPCODE:
      type_ = R_TYPE_INSN;
      code_opless_ = code & ((addr_t) func7_ << 25 | func3_ << 12 | opcode_);
      break;

    case RV_I_TYPE_OPCODE:
    case RV_ILOAD_TYPE_OPCODE:
    case RV_IJALR_TYPE_OPCODE:
      type_ = I_TYPE_INSN;
      // todo decode
      break;
    case RV_S_TYPE_OPCODE:
    case RV_SB_TYPE_OPCODE:
      type_ = S_TYPE_INSN;
      // todo decode
      break;
    case RV_U1_TYPE_OPCODE:
    case RV_U2_TYPE_OPCODE:
      type_ = U_TYPE_INSN;
      // todo decode
      break;
    default:
      type_ = UNDEF_TYPE_INSN;
      // todo decode to hlt?
    }
  }

  uint8_t opcode() const { return opcode_; }
  addr_t code() const { return code_; }
  addr_t code_opless() const { return code_opless_; }
  RVInsnType type() const { return type_; }

  uint8_t rd()    const { return rd_; }
  uint8_t rs1()   const { return rs1_; }
  uint8_t rs2()   const { return rs2_; }
  uint8_t func3() const { return func3_; }
  uint8_t func7() const { return func7_; }

  uint16_t imm11_0()  const { return imm11_0_; }
  uint8_t  imm11_5()  const { return imm11_5_; }
  uint8_t  imm4_0()   const { return imm4_0_; };
  uint32_t imm31_12() const { return imm31_12_; }

  void print(std::ostream &out) const {
    switch (type_)
    {
    case R_TYPE_INSN:
      out << std::bitset<7>{func7_} << "'"
          << std::bitset<5>{rs2_} << "'"
          << std::bitset<5>{rs1_} << "'"
          << std::bitset<3>{func3_} << "'"
          << std::bitset<5>{rd_} << "'"
          << std::bitset<7>{opcode_} << " (R)";
      break;
    case UNDEF_TYPE_INSN:
      out << std::bitset<32>{code_} << " (U)";
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

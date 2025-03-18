#ifndef ISA_HPP
#define ISA_HPP

#include <iostream>

#include "instruction.hpp"

namespace rv32i_sim {

class RTypeInsn : public RVInsn {

public:
  RTypeInsn(addr_t code) : RVInsn{code, RVInsnType::R_TYPE_INSN} {
    addOperand(
      Operand::createEnc("opcode",
        static_cast<addr_t>(code & DEFAULT_OPCODE_MASK)
      )
    );

    addOperand(
      Operand::createReg("rd",
        static_cast<Register>((code_ >> 7) & ((1 << 5) - 1))
      )
    );

    addOperand(
      Operand::createEnc("func3",
        static_cast<addr_t>((code_ >> 12) & ((1 << 3) - 1))
      )
    );

    addOperand(
      Operand::createReg("rs1",
        static_cast<Register>((code_ >> 15) & ((1 << 5) - 1))
      )
    );

    addOperand(
      Operand::createReg("rs2",
        static_cast<Register>((code_ >> 20) & ((1 << 5) - 1))
      )
    );

    addOperand(
      Operand::createEnc("func7",
        static_cast<addr_t>((code_ >> 25) & ((1 << 7) - 1))
      )
    );

    opcode_ = RTypeInsn::getOpcode(code_);
  }

  void print(std::ostream& out) const override {
    out << std::bitset<7>{static_cast<uint8_t>(getOperand(5).getEnc())} << "'"
        << std::bitset<5>{static_cast<uint8_t>(getOperand(4).getReg())} << "'"
        << std::bitset<5>{static_cast<uint8_t>(getOperand(3).getReg())} << "'"
        << std::bitset<3>{static_cast<uint8_t>(getOperand(2).getEnc())} << "'"
        << std::bitset<5>{static_cast<uint8_t>(getOperand(1).getReg())} << "'"
        << std::bitset<7>{static_cast<uint8_t>(getOperand(0).getEnc())} << " (R)";
 }

  static addr_t getOpcode(addr_t code) {
    addr_t func_7 = code & DEFAULT_FUNC7_MASK;
    addr_t func_3 = code & DEFAULT_FUNC3_MASK;
    addr_t opcode_7_0 = code & DEFAULT_OPCODE_MASK;

    return func_7 | func_3 | opcode_7_0;
  }

  static RTypeInsn* decode(addr_t code);

  virtual ~RTypeInsn() = default;
};

class rvADD final : public RTypeInsn {
public:
  rvADD(addr_t code) : RTypeInsn(code) {}

  void execute(IRVModel& model) const override;
};

class rvSUB final : public RTypeInsn {
public:
  rvSUB(addr_t code) : RTypeInsn(code) {}

  void execute(IRVModel& model) const override;
};

class rvSLL final : public RTypeInsn {
public:
  rvSLL(addr_t code) : RTypeInsn(code) {}

  void execute(IRVModel& model) const override;
};

class rvSLT final : public RTypeInsn {
public:
  rvSLT(addr_t code) : RTypeInsn(code) {}

  void execute(IRVModel& model) const override;
};

class rvSLTU final : public RTypeInsn {
public:
  rvSLTU(addr_t code) : RTypeInsn(code) {}

  void execute(IRVModel& model) const override;
};

class rvXOR final : public RTypeInsn {
public:
  rvXOR(addr_t code) : RTypeInsn(code) {}

  void execute(IRVModel& model) const override;
};

class rvSRL final : public RTypeInsn {
public:
  rvSRL(addr_t code) : RTypeInsn(code) {}

  void execute(IRVModel& model) const override;
};

class rvSRA final : public RTypeInsn {
public:
  rvSRA(addr_t code) : RTypeInsn(code) {}

  void execute(IRVModel& model) const override;
};

class rvOR final : public RTypeInsn {
public:
  rvOR(addr_t code) : RTypeInsn(code) {}

  void execute(IRVModel& model) const override;
};

class rvAND final : public RTypeInsn {
public:
  rvAND(addr_t code) : RTypeInsn(code) {}

  void execute(IRVModel& model) const override;
};

class rvUNDEF final : public RTypeInsn {
public:
  rvUNDEF(addr_t code) : RTypeInsn{code} {}

  void execute(IRVModel& model) const override;
};

RTypeInsn* RTypeInsn::decode(addr_t code) {
  switch (static_cast<RV32i_ISA>(RTypeInsn::getOpcode(code)))
  {
  case RV32i_ISA::ADD: return new rvADD{code};
  case RV32i_ISA::SUB: return new rvSUB{code};
  case RV32i_ISA::SLL: return new rvSLL{code};
  case RV32i_ISA::SLT: return new rvSLT{code};
  case RV32i_ISA::SLTU: return new rvSLTU{code};
  case RV32i_ISA::XOR: return new rvXOR{code};
  case RV32i_ISA::SRL: return new rvSRL{code};
  case RV32i_ISA::SRA: return new rvSRA{code};
  case RV32i_ISA::OR: return new rvOR{code};
  case RV32i_ISA::AND: return new rvAND{code};
  default: return new rvUNDEF{code};
  }
}

RVInsn* RVInsn::decode(addr_t code) {
  addr_t opcode = code & DEFAULT_OPCODE_MASK;
  switch (opcode)
  {
  case RV_R_TYPE_OPCODE: return RTypeInsn::decode(code);

  // case RV_I_TYPE_OPCODE:
  // case RV_IJALR_TYPE_OPCODE:
  // case RV_ILOAD_TYPE_OPCODE:
    // return ITypeInsn::decode(code);

  // case RV_S_TYPE_OPCODE:
  // case RV_SB_TYPE_OPCODE:
    // return STypeInsn::decode(code);

  // case RV_U1_TYPE_OPCODE:
  // case RV_U2_TYPE_OPCODE:
    // return UTypeInsn::decode(code);

  // default:
    // return UndefTypeInsn::decode(code);
  default:
    return new GeneralUndefInsn{code};
    break;
  }
}

} // rv32i_sim

#endif // ISA_HPP

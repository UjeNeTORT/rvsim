#ifndef ISA_HPP
#define ISA_HPP

#include <iostream>

#include "instruction.hpp"

namespace rv32i_sim {

class RTypeInsn : public RVInsnNew {

public:
  RTypeInsn(addr_t code) : RVInsnNew{code} {
    addOperand(
      Operand::createReg(
        static_cast<Register>((code_ >> 7) & ((1 << 5)  - 1))
      )
    );

    addOperand(
      Operand::createReg(
        static_cast<Register>((code_ >> 15) & ((1 << 5)  - 1))
      )
    );

    addOperand(
      Operand::createReg(
        static_cast<Register>((code_ >> 20) & ((1 << 5)  - 1))
      )
    );

    opcode_ = RTypeInsn::getOpcode(code_);
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

  void execute(RVModel& model) const override {
    std::cerr << "executing add insn\n";
  }
};

class rvSUB final : public RTypeInsn {
public:
  rvSUB(addr_t code) : RTypeInsn(code) {}

  void execute(RVModel& model) const override {
    std::cerr << "executing sub insn\n";
  }
};

class rvSLL final : public RTypeInsn {
public:
  rvSLL(addr_t code) : RTypeInsn(code) {}

  void execute(RVModel& model) const override {
    std::cerr << "executing sll insn\n";
  }
};

class rvSLT final : public RTypeInsn {
public:
  rvSLT(addr_t code) : RTypeInsn(code) {}

  void execute(RVModel& model) const override {
    std::cerr << "executing slt insn\n";
  }
};

class rvSLTU final : public RTypeInsn {
public:
  rvSLTU(addr_t code) : RTypeInsn(code) {}

  void execute(RVModel& model) const override {
    std::cerr << "executing sltu insn\n";
  }
};

class rvXOR final : public RTypeInsn {
public:
  rvXOR(addr_t code) : RTypeInsn(code) {}

  void execute(RVModel& model) const override {
    std::cerr << "executing xor insn\n";
  }
};

class rvSRL final : public RTypeInsn {
public:
  rvSRL(addr_t code) : RTypeInsn(code) {}

  void execute(RVModel& model) const override {
    std::cerr << "executing srl insn\n";
  }
};

class rvSRA final : public RTypeInsn {
public:
  rvSRA(addr_t code) : RTypeInsn(code) {}

  void execute(RVModel& model) const override {
    std::cerr << "executing sra insn\n";
  }
};

class rvOR final : public RTypeInsn {
public:
  rvOR(addr_t code) : RTypeInsn(code) {}

  void execute(RVModel& model) const override {
    std::cerr << "executing or insn\n";
  }
};

class rvAND final : public RTypeInsn {
public:
  rvAND(addr_t code) : RTypeInsn(code) {}

  void execute(RVModel& model) const override {
    std::cerr << "executing and insn\n";
  }
};

class rvNOP final : public RVInsnNew {
public:
  rvNOP() : RVInsnNew{0} {}

  // todo nop encoding
  addr_t getCode() const override { return 0; }
  void setCode(addr_t code) override { return; };

  // todo nop encoding
  addr_t getOpcode() const override { return 0; };

  void execute(RVModel& model) const override {
    std::cerr << "nop insn\n";
  }
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
  default: return nullptr; // todo undefined instruction
  }
}

RVInsnNew* RVInsnNew::decode(addr_t code) {
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
    std::cerr << "undef insn\n";
    return new rvNOP{};
    break;
  }
}

} // rv32i_sim

#endif // ISA_HPP

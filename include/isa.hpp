#ifndef ISA_HPP
#define ISA_HPP

#include <iostream>
#include <memory>

#include "instruction.hpp"

/**
 * RISCV ISA MANUAL:
 * https://drive.google.com/file/d/1uviu1nH-tScFfgrovvFCrj7Omv8tFtkp/view
*/

namespace rv32i_sim {

class RTypeInsn : public RVInsn {
protected:
  Register dst_ = Register::INVALID;
  Register rs1_ = Register::INVALID;
  Register rs2_ = Register::INVALID;

public:
  RTypeInsn(addr_t code, std::string name = "???") :
                                    RVInsn{code, RVInsnType::R_TYPE_INSN, name} {
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

    dst_ = getOperand(1).getReg();
    rs1_ = getOperand(3).getReg();
    rs2_ = getOperand(4).getReg();
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

  static std::unique_ptr<RTypeInsn> decode(addr_t code);

  virtual ~RTypeInsn() = default;
};

class rvADD final : public RTypeInsn {
public:
  rvADD(addr_t code) : RTypeInsn(code, "add") {}

  void execute(IRVModel& model) const override;
};

class rvSUB final : public RTypeInsn {
public:
  rvSUB(addr_t code) : RTypeInsn(code, "sub") {}

  void execute(IRVModel& model) const override;
};

class rvSLL final : public RTypeInsn {
public:
  rvSLL(addr_t code) : RTypeInsn(code, "sll") {}

  void execute(IRVModel& model) const override;
};

class rvSLT final : public RTypeInsn {
public:
  rvSLT(addr_t code) : RTypeInsn(code, "slt") {}

  void execute(IRVModel& model) const override;
};

class rvSLTU final : public RTypeInsn {
public:
  rvSLTU(addr_t code) : RTypeInsn(code, "sltu") {}

  void execute(IRVModel& model) const override;
};

class rvXOR final : public RTypeInsn {
public:
  rvXOR(addr_t code) : RTypeInsn(code, "xor") {}

  void execute(IRVModel& model) const override;
};

class rvSRL final : public RTypeInsn {
public:
  rvSRL(addr_t code) : RTypeInsn(code, "srl") {}

  void execute(IRVModel& model) const override;
};

class rvSRA final : public RTypeInsn {
public:
  rvSRA(addr_t code) : RTypeInsn(code, "sra") {}

  void execute(IRVModel& model) const override;
};

class rvOR final : public RTypeInsn {
public:
  rvOR(addr_t code) : RTypeInsn(code, "or") {}

  void execute(IRVModel& model) const override;
};

class rvAND final : public RTypeInsn {
public:
  rvAND(addr_t code) : RTypeInsn(code, "and") {}

  void execute(IRVModel& model) const override;
};

class rvUNDEF_R final : public RTypeInsn {
public:
  rvUNDEF_R(addr_t code) : RTypeInsn{code} {}

  void execute(IRVModel& model) const override;
};

std::unique_ptr<RTypeInsn> RTypeInsn::decode(addr_t code) {
  switch (static_cast<RV32i_ISA>(RTypeInsn::getOpcode(code)))
  {
  case RV32i_ISA::ADD: return std::make_unique<rvADD>(code);
  case RV32i_ISA::SUB: return std::make_unique<rvSUB>(code);
  case RV32i_ISA::SLL: return std::make_unique<rvSLL>(code);
  case RV32i_ISA::SLT: return std::make_unique<rvSLT>(code);
  case RV32i_ISA::SLTU: return std::make_unique<rvSLTU>(code);
  case RV32i_ISA::XOR: return std::make_unique<rvXOR>(code);
  case RV32i_ISA::SRL: return std::make_unique<rvSRL>(code);
  case RV32i_ISA::SRA: return std::make_unique<rvSRA>(code);
  case RV32i_ISA::OR: return std::make_unique<rvOR>(code);
  case RV32i_ISA::AND: return std::make_unique<rvAND>(code);
  default: return std::make_unique<rvUNDEF_R>(code);
  }
}

class ITypeInsn : public RVInsn {
protected:
  Register rd_ = Register::INVALID;
  Register rs1_ = Register::INVALID;
  uint32_t imm_ = 0;

public:
  ITypeInsn(addr_t code = 0, std::string name = "???") :
                                    RVInsn{code, RVInsnType::I_TYPE_INSN, name} {
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
      Operand::createImm("imm[11:0]",
        static_cast<addr_t>((code_ >> 20) & ((1 << 12) - 1))
      )
    );

    opcode_ = ITypeInsn::getOpcode(code_);

    rd_ = getOperand(1).getReg();
    rs1_ = getOperand(3).getReg();
    imm_ = getOperand(4).getImm();
  }

  void print(std::ostream& out) const override {
    out << std::bitset<12>{static_cast<uint16_t>(getOperand(4).getImm())} << "'"
        << std::bitset<5>{static_cast<uint8_t>(getOperand(3).getReg())} << "'"
        << std::bitset<3>{static_cast<uint8_t>(getOperand(2).getEnc())} << "'"
        << std::bitset<5>{static_cast<uint8_t>(getOperand(1).getReg())} << "'"
        << std::bitset<7>{static_cast<uint8_t>(getOperand(0).getEnc())} << " (I)";
 }

  static addr_t getOpcode(addr_t code) {
    addr_t opcode_7_0 = code & DEFAULT_OPCODE_MASK;
    addr_t func_3 = code & DEFAULT_FUNC3_MASK;
    addr_t srai_specific = code & MASK_31_25;

    if ((srai_specific | func_3 | opcode_7_0) == static_cast<addr_t>(RV32i_ISA::SRAI))
      return static_cast<addr_t>(RV32i_ISA::SRAI);

    addr_t func_12 = code & MASK_31_20; // system insns specific

    if ((func_12 | func_3 | opcode_7_0) == static_cast<addr_t>(RV32i_ISA::EBREAK))
      return static_cast<addr_t>(RV32i_ISA::EBREAK);

    return func_3 | opcode_7_0;
  }

  static std::unique_ptr<ITypeInsn> decode(addr_t code);

  virtual ~ITypeInsn() = default;
};

class rvJALR final : public ITypeInsn {
public:
  rvJALR(addr_t code) : ITypeInsn(code, "jalr") {}

  void execute(IRVModel& model) const override;
};

class rvLB final : public ITypeInsn {
public:
  rvLB(addr_t code) : ITypeInsn(code, "lb") {}

  void execute(IRVModel& model) const override;
};

class rvLH final : public ITypeInsn {
public:
  rvLH(addr_t code) : ITypeInsn(code, "lh") {}

  void execute(IRVModel& model) const override;
};

class rvLW final : public ITypeInsn {
public:
  rvLW(addr_t code) : ITypeInsn(code, "lw") {}

  void execute(IRVModel& model) const override;
};

class rvLBU final : public ITypeInsn {
public:
  rvLBU(addr_t code) : ITypeInsn(code, "lbu") {}

  void execute(IRVModel& model) const override;
};

class rvLHU final : public ITypeInsn {
public:
  rvLHU(addr_t code) : ITypeInsn(code, "lhu") {}

  void execute(IRVModel& model) const override;
};

class rvADDI final : public ITypeInsn {
public:
  rvADDI(addr_t code) : ITypeInsn(code, "addi") {}

  void execute(IRVModel& model) const override;
};

class rvSLTI final : public ITypeInsn {
public:
  rvSLTI(addr_t code) : ITypeInsn(code, "slti") {}

  void execute(IRVModel& model) const override;
};

class rvSLTIU final : public ITypeInsn {
public:
  rvSLTIU(addr_t code) : ITypeInsn(code, "sltiu") {}

  void execute(IRVModel& model) const override;
};

class rvXORI final : public ITypeInsn {
public:
  rvXORI(addr_t code) : ITypeInsn(code, "xori") {}

  void execute(IRVModel& model) const override;
};

class rvORI final : public ITypeInsn {
public:
  rvORI(addr_t code) : ITypeInsn(code, "ori") {}

  void execute(IRVModel& model) const override;
};

class rvANDI final : public ITypeInsn {
public:
  rvANDI(addr_t code) : ITypeInsn(code, "andi") {}

  void execute(IRVModel& model) const override;
};

class rvSLLI final : public ITypeInsn {
public:
  rvSLLI(addr_t code) : ITypeInsn(code, "slli") {}

  void execute(IRVModel& model) const override;
};

class rvSRLI final : public ITypeInsn {
public:
  rvSRLI(addr_t code) : ITypeInsn(code, "srli") {}

  void execute(IRVModel& model) const override;
};

class rvSRAI final : public ITypeInsn {
public:
  rvSRAI(addr_t code) : ITypeInsn(code, "srai") {}

  void execute(IRVModel& model) const override;
};

class rvEBREAK final : public ITypeInsn {
public:
  rvEBREAK() {
    code_ = static_cast<addr_t>(RV32i_ISA::EBREAK);
    opcode_ = code_ & DEFAULT_OPCODE_MASK;
    name_ = "ebreak";

    addOperand(
      Operand::createEnc("opcode",
        static_cast<addr_t>(opcode_)
      )
    );
  }

  rvEBREAK(addr_t code) : ITypeInsn(code, "ebreak") {}

  void execute(IRVModel& model) const override;
};

class rvECALL final : public ITypeInsn {
public:
  rvECALL() {
    code_ = static_cast<addr_t>(RV32i_ISA::ECALL);
    opcode_ = code_ & DEFAULT_OPCODE_MASK;
    name_ = "ecall";

    addOperand(
      Operand::createEnc("opcode",
        static_cast<addr_t>(opcode_)
      )
    );
  }

  rvECALL(addr_t code) : ITypeInsn(code, "ecall") {}

  void execute(IRVModel& model) const override;
};

class rvUNDEF_I final : public ITypeInsn {
public:
  rvUNDEF_I(addr_t code) : ITypeInsn(code) {}

  void execute(IRVModel& model) const override;
};

std::unique_ptr<ITypeInsn> ITypeInsn::decode(addr_t code) {
  switch (static_cast<RV32i_ISA>(ITypeInsn::getOpcode(code)))
  {
  case RV32i_ISA::JALR: return std::make_unique<rvJALR>(code);
  case RV32i_ISA::LB: return std::make_unique<rvLB>(code);
  case RV32i_ISA::LH: return std::make_unique<rvLH>(code);
  case RV32i_ISA::LW: return std::make_unique<rvLW>(code);
  case RV32i_ISA::LBU: return std::make_unique<rvLBU>(code);
  case RV32i_ISA::LHU: return std::make_unique<rvLHU>(code);
  case RV32i_ISA::ADDI: return std::make_unique<rvADDI>(code);
  case RV32i_ISA::SLTI: return std::make_unique<rvSLTI>(code);
  case RV32i_ISA::SLTIU: return std::make_unique<rvSLTIU>(code);
  case RV32i_ISA::XORI: return std::make_unique<rvXORI>(code);
  case RV32i_ISA::ORI: return std::make_unique<rvORI>(code);
  case RV32i_ISA::ANDI: return std::make_unique<rvANDI>(code);
  case RV32i_ISA::SLLI: return std::make_unique<rvSLLI>(code);
  case RV32i_ISA::SRLI: return std::make_unique<rvSRLI>(code);
  case RV32i_ISA::SRAI: return std::make_unique<rvSRAI>(code);
  case RV32i_ISA::EBREAK: return std::make_unique<rvEBREAK>(code);
  case RV32i_ISA::ECALL: return std::make_unique<rvECALL>(code);
  default: return std::make_unique<rvUNDEF_I>(code);
  }
}

class STypeInsn : public RVInsn {
protected:
  Register rs1_ = Register::INVALID;
  Register rs2_ = Register::INVALID;
  uint32_t imm_ = 0;

public:
  STypeInsn(addr_t code, std::string name = "???") :
                        RVInsn{code, RVInsnType::S_TYPE_INSN, name} {
    addOperand(
      Operand::createEnc("opcode",
        static_cast<addr_t>(code & DEFAULT_OPCODE_MASK)
      )
    );

    addOperand(
      Operand::createImm("imm[4:0]",
        static_cast<addr_t>((code_ >> 7) & ((1 << 5) - 1))
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
      Operand::createImm("imm[11:5]",
        static_cast<addr_t>((code_ >> 25) & ((1 << 7) - 1))
      )
    );

    opcode_ = STypeInsn::getOpcode(code_);

    rs1_ = getOperand(3).getReg();
    rs2_ = getOperand(4).getReg();
    imm_ = (getOperand(5).getImm() << 5) | getOperand(1).getImm();
  }

  void print(std::ostream& out) const override {
    out << std::bitset<7>{static_cast<uint8_t>(getOperand(5).getImm())} << "'"
        << std::bitset<5>{static_cast<uint8_t>(getOperand(4).getReg())} << "'"
        << std::bitset<5>{static_cast<uint8_t>(getOperand(3).getReg())} << "'"
        << std::bitset<3>{static_cast<uint8_t>(getOperand(2).getEnc())} << "'"
        << std::bitset<5>{static_cast<uint8_t>(getOperand(1).getImm())} << "'"
        << std::bitset<7>{static_cast<uint8_t>(getOperand(0).getEnc())} << " (S)";
 }

  static addr_t getOpcode(addr_t code) {
    addr_t opcode_7_0 = code & DEFAULT_OPCODE_MASK;
    addr_t func_3 = code & DEFAULT_FUNC3_MASK;

    return func_3 | opcode_7_0;
  }

  static std::unique_ptr<STypeInsn> decode(addr_t code);

  virtual ~STypeInsn() = default;
};

class rvSB final : public STypeInsn {
public:
  rvSB(addr_t code) : STypeInsn(code, "sb") {}

  void execute(IRVModel& model) const override;
};

class rvSH final : public STypeInsn {
public:
  rvSH(addr_t code) : STypeInsn(code, "sh") {}

  void execute(IRVModel& model) const override;
};

class rvSW final : public STypeInsn {
public:
  rvSW(addr_t code) : STypeInsn(code, "sw") {}

  void execute(IRVModel& model) const override;
};

class rvUNDEF_S final : public STypeInsn {
public:
  rvUNDEF_S(addr_t code) : STypeInsn(code) {}

  void execute(IRVModel& model) const override;
};

std::unique_ptr<STypeInsn> STypeInsn::decode(addr_t code) {
  switch (static_cast<RV32i_ISA>(STypeInsn::getOpcode(code)))
  {
  case RV32i_ISA::SB: return std::make_unique<rvSB>(code);
  case RV32i_ISA::SH: return std::make_unique<rvSH>(code);
  case RV32i_ISA::SW: return std::make_unique<rvSW>(code);
  default: return std::make_unique<rvUNDEF_S>(code);
  }
}

class BTypeInsn : public RVInsn {
protected:
  Register rs1_ = Register::INVALID;
  Register rs2_ = Register::INVALID;
  uint32_t imm_ = 0;

public:
  BTypeInsn(addr_t code, std::string name = "???") :
                        RVInsn{code, RVInsnType::B_TYPE_INSN, name} {
    addOperand(
      Operand::createEnc("opcode",
        static_cast<addr_t>(code & DEFAULT_OPCODE_MASK)
      )
    );

    addOperand(
      Operand::createImm("imm[11]",
        static_cast<addr_t>((code_ >> 7) & 1) // single bit
      )
    );

    addOperand(
      Operand::createImm("imm[4:1]",
        static_cast<addr_t>((code_ >> 7) & ((1 << 4) - 1))
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
      Operand::createImm("imm[10:5]",
        static_cast<addr_t>((code_ >> 25) & ((1 << 7) - 1))
      )
    );

    addOperand(
      Operand::createImm("imm[12]",
        static_cast<addr_t>((code_ >> 31) & 1) // single bit
      )
    );

    opcode_ = BTypeInsn::getOpcode(code_);

    rs1_ = getOperand(4).getReg();
    rs2_ = getOperand(5).getReg();
    addr_t bit_4_1 = getOperand(2).getImm() << 1;
    addr_t bit_10_5 = getOperand(6).getImm() << 5;
    addr_t bit_11 = getOperand(1).getImm() << 11;
    addr_t bit_12 = getOperand(7).getImm() << 12;

    imm_ = bit_12 | bit_11 | bit_10_5 | bit_4_1 | 0; // todo sing extend? (everywhere?)
  }

  void print(std::ostream& out) const override {
    out << std::bitset<1>{static_cast<uint8_t>(getOperand(7).getImm())} << "'"
        << std::bitset<6>{static_cast<uint8_t>(getOperand(6).getImm())} << "'"
        << std::bitset<5>{static_cast<uint8_t>(getOperand(5).getReg())} << "'"
        << std::bitset<5>{static_cast<uint8_t>(getOperand(4).getReg())} << "'"
        << std::bitset<3>{static_cast<uint8_t>(getOperand(3).getEnc())} << "'"
        << std::bitset<4>{static_cast<uint8_t>(getOperand(2).getImm())} << "'"
        << std::bitset<1>{static_cast<uint8_t>(getOperand(1).getImm())} << "'"
        << std::bitset<7>{static_cast<uint8_t>(getOperand(0).getEnc())} << " (B)";
 }

  static addr_t getOpcode(addr_t code) {
    addr_t opcode_7_0 = code & DEFAULT_OPCODE_MASK;
    addr_t func_3 = code & DEFAULT_FUNC3_MASK;

    return func_3 | opcode_7_0;
  }

  static std::unique_ptr<BTypeInsn> decode(addr_t code);

  virtual ~BTypeInsn() = default;
};

class rvBEQ final : public BTypeInsn {
public:
  rvBEQ(addr_t code) : BTypeInsn(code, "beq") {}

  void execute(IRVModel& model) const override;
};

class rvBNE final : public BTypeInsn {
public:
  rvBNE(addr_t code) : BTypeInsn(code, "bne") {}

  void execute(IRVModel& model) const override;
};

class rvBLT final : public BTypeInsn {
public:
  rvBLT(addr_t code) : BTypeInsn(code, "blt") {}

  void execute(IRVModel& model) const override;
};

class rvBLTU final : public BTypeInsn {
public:
  rvBLTU(addr_t code) : BTypeInsn(code, "bltu") {}

  void execute(IRVModel& model) const override;
};

class rvBGE final : public BTypeInsn {
public:
  rvBGE(addr_t code) : BTypeInsn(code, "bge") {}

  void execute(IRVModel& model) const override;
};

class rvBGEU final  : public BTypeInsn {
public:
  rvBGEU(addr_t code) : BTypeInsn(code, "bgeu") {}

  void execute(IRVModel& model) const override;
};

class rvUNDEF_B final : public BTypeInsn {
public:
  rvUNDEF_B(addr_t code) : BTypeInsn(code) {}

  void execute(IRVModel& model) const override;
};

std::unique_ptr<BTypeInsn> BTypeInsn::decode(addr_t code) {
  switch (static_cast<RV32i_ISA>(BTypeInsn::getOpcode(code)))
  {
  case RV32i_ISA::BEQ: return std::make_unique<rvBEQ>(code);
  case RV32i_ISA::BNE: return std::make_unique<rvBNE>(code);
  case RV32i_ISA::BLT: return std::make_unique<rvBLT>(code);
  case RV32i_ISA::BLTU: return std::make_unique<rvBLTU>(code);
  case RV32i_ISA::BGE: return std::make_unique<rvBGE>(code);
  case RV32i_ISA::BGEU: return std::make_unique<rvBGEU>(code);
  default: return std::make_unique<rvUNDEF_B>(code);
  }
}

class UTypeInsn : public RVInsn {
protected:
  Register rd_ = Register::INVALID;
  uint32_t imm_ = 0;

public:
  UTypeInsn(addr_t code, std::string name = "???") :
                            RVInsn{code, RVInsnType::U_TYPE_INSN, name} {
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
      Operand::createImm("imm[31:12]",
        static_cast<addr_t>(sword_t(code_) >> 12) // arithmetic shift right
      )
    );

    opcode_ = BTypeInsn::getOpcode(code_);

    rd_ = getOperand(1).getReg();
    imm_ = getOperand(2).getImm() << 12;
  }

  void print(std::ostream& out) const override {
    out << std::bitset<20>{static_cast<uint32_t>(getOperand(2).getImm())} << "'"
        << std::bitset<5>{static_cast<uint8_t>(getOperand(1).getReg())} << "'"
        << std::bitset<7>{static_cast<uint8_t>(getOperand(0).getEnc())} << " (U)";
 }

  static addr_t getOpcode(addr_t code) {
    addr_t opcode_7_0 = code & DEFAULT_OPCODE_MASK;
    return opcode_7_0;
  }

  static std::unique_ptr<UTypeInsn> decode(addr_t code);

  virtual ~UTypeInsn() = default;
};

class rvLUI : public UTypeInsn {
public:
  rvLUI(addr_t code) : UTypeInsn(code, "lui") {}

  void execute(IRVModel& model) const override;
};

class rvAUIPC : public UTypeInsn {
public:
  rvAUIPC(addr_t code) : UTypeInsn(code, "auipc") {}

  void execute(IRVModel& model) const override;
};

class rvUNDEF_U : public UTypeInsn {
public:
  rvUNDEF_U(addr_t code) : UTypeInsn(code) {}

  void execute(IRVModel& model) const override;
};

std::unique_ptr<UTypeInsn> UTypeInsn::decode(addr_t code) {
  switch (static_cast<RV32i_ISA>(UTypeInsn::getOpcode(code)))
  {
  case RV32i_ISA::LUI: return std::make_unique<rvLUI>(code);
  case RV32i_ISA::AUIPC: return std::make_unique<rvAUIPC>(code);
  default: return std::make_unique<rvUNDEF_U>(code);
  }
}

class rvJAL : public RVInsn {
  Register rd_ = Register::INVALID;
  addr_t imm_ = 0;

public:
  rvJAL() {
    code_ = static_cast<addr_t>(RV32i_ISA::JAL);
    opcode_ = code_ & DEFAULT_OPCODE_MASK;
    type_ = RVInsnType::J_TYPE_INSN;
    name_ = "jal";

    addOperand(
      Operand::createEnc("opcode",
        static_cast<addr_t>(opcode_)
      )
    );
  }

  rvJAL(addr_t code) : RVInsn(code, RVInsnType::J_TYPE_INSN, "jal") {
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
      Operand::createImm("imm[19:12]",
        static_cast<addr_t>((code_ >> 12) & ((1 << 8) - 1))
      )
    );

    addOperand(
      Operand::createImm("imm[11]",
        static_cast<addr_t>((code_ >> 20) & 1) // single bit
      )
    );

    addOperand(
      Operand::createImm("imm[10:1]",
        static_cast<addr_t>((code_ >> 21) & ((1 << 10) - 1))
      )
    );

    addOperand(
      Operand::createImm("imm[20]",
        static_cast<addr_t>((code_ >> 31) & 1) // single bit
      )
    );

    rd_ = getOperand(1).getReg();

    addr_t bit_20     = getOperand(5).getImm() << 20;
    addr_t bits_19_12 = getOperand(2).getImm() << 12;
    addr_t bit_11     = getOperand(3).getImm() << 11;
    addr_t bits_10_1  = getOperand(4).getImm() << 1;

    imm_ = bit_20 | bits_19_12 | bit_11 | bits_10_1 | 0;
  }

  static addr_t getOpcode(addr_t code) {
    return code & DEFAULT_OPCODE_MASK;
  }

  // todo refactor + implement for other classes
  addr_t encode(Register reg, sword_t imm) {
    rd_ = reg;
    imm_ = std::bit_cast<addr_t>(imm);

    addr_t bit_20     = (imm_ & (1 << 20))  >> 20;
    addr_t bits_19_12 = (imm_ & MASK_19_12) >> 12;
    addr_t bit_11     = (imm_ & (1 << 11))  >> 11;
    addr_t bits_10_1  = (imm_ & MASK_10_1)  >> 1;

    code_ = opcode_
      | (static_cast<addr_t>(reg) << 7)
      | (bits_19_12 << 12)
      | (bit_11 << 20)
      | (bits_10_1 << 21)
      | (bit_20 << 31);

    addOperand(
      Operand::createReg("rd", reg)
    );

    addOperand(
      Operand::createImm("imm[19:12]", bits_19_12)
    );

    addOperand(
      Operand::createImm("imm[11]", bit_11)
    );

    addOperand(
      Operand::createImm("imm[10:1]", bits_10_1)
    );

    addOperand(
      Operand::createImm("imm[20]", bit_20)
    );

    return code_;
  }

  void execute(IRVModel& model) const override;

  void print(std::ostream& out) const override {
    out << std::bitset<1>{static_cast<uint8_t>(getOperand(5).getImm())} << "'"
        << std::bitset<10>{static_cast<uint16_t>(getOperand(4).getImm())} << "'"
        << std::bitset<1>{static_cast<uint8_t>(getOperand(3).getImm())} << "'"
        << std::bitset<8>{static_cast<uint8_t>(getOperand(2).getImm())} << "'"
        << std::bitset<5>{static_cast<uint8_t>(getOperand(1).getReg())} << "'"
        << std::bitset<7>{static_cast<uint8_t>(getOperand(0).getEnc())} << " (J)";
  }
};

std::unique_ptr<RVInsn> RVInsn::decode(addr_t code) {
  addr_t opcode = code & DEFAULT_OPCODE_MASK;
  switch (opcode)
  {
  case RV_R_TYPE_OPCODE:
    return RTypeInsn::decode(code);

  case RV_I_TYPE_OPCODE:
  case RV_IJALR_TYPE_OPCODE:
  case RV_ILOAD_TYPE_OPCODE:
  case RV_SYSTEM_I_OPCODE:
    return ITypeInsn::decode(code);

  case RV_S_TYPE_OPCODE:
    return STypeInsn::decode(code);

  case RV_B_TYPE_OPCODE:
    return BTypeInsn::decode(code);

  case RV_U1_TYPE_OPCODE:
  case RV_U2_TYPE_OPCODE:
    return UTypeInsn::decode(code);

  case RV_JAL_OPCODE:
    return std::make_unique<rvJAL>(code);

  default:
    return std::make_unique<GeneralUndefInsn>(code);
  }
}

} // rv32i_sim

#endif // ISA_HPP

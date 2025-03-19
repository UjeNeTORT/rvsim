#ifndef INSTRUCTION_HPP
#define INSTRUCTION_HPP

#include <bitset>
#include <cassert>
#include <variant>

#include "encoding.hpp"

namespace rv32i_sim {

class RVModel;

class Operand {
  enum class OpType : uint8_t {
    INVALID = 0,
    REG = 1, // a register encoded in an insn
    IMM = 2, //< an immediate encoded in an insn
    ENC = 3, //< part of encoding which is not actually an operand
  };

  std::variant<Register, addr_t> op_;
  std::string name_ = "???";
  OpType type_ = OpType::INVALID;

public:
  bool isValid() const { return type_ != OpType::INVALID; }
  bool isReg() const { return type_ == OpType::REG; }
  bool isImm() const { return type_ == OpType::IMM; }
  bool isEnc() const { return type_ == OpType::ENC; }

  Register getReg() const {
    assert(std::holds_alternative<Register>(op_) && "Variant holds another alternative");
    assert(isReg() && "Type mismatch");
    return std::get<Register>(op_);
  }

  addr_t getImm() const {
    assert(std::holds_alternative<uint32_t>(op_) && "Variant holds another alternative");
    assert(isImm() && "Type mismatch");
    return std::get<addr_t>(op_);
  }

  addr_t getEnc() const {
    assert(std::holds_alternative<addr_t>(op_) && "Variant holds another alternative");
    assert(isEnc() && "Type mismatch");
    return std::get<addr_t>(op_);
  }

  void setReg(Register reg) {
    assert(std::holds_alternative<Register>(op_) && "Variant holds another alternative");
    assert(isReg() && "Type mismatch");
    op_ = reg;
  }

  void setImm(addr_t imm) {
    assert(std::holds_alternative<uint32_t>(op_) && "Variant holds another alternative");
    assert(isImm() && "Type mismatch");
    op_ = imm;
  }

  void setEnc(addr_t enc) {
    assert(std::holds_alternative<addr_t>(op_) && "Variant holds another alternative");
    assert(isEnc() && "Type mismatch");
    op_ = enc;
  }

  static Operand createReg(std::string name, Register reg) {
    assert(isRegValid(reg) && "Invalid register");

    Operand op;
    op.type_ = OpType::REG;
    op.op_ = reg;
    op.name_ = name;
    return op;
  }

  static Operand createImm(std::string name, uint32_t imm) {
    Operand op;
    op.type_ = OpType::IMM;
    op.op_ = imm;
    op.name_ = name;
    return op;
  }

  static Operand createEnc(std::string name, addr_t enc) {
    Operand op;
    op.type_ = OpType::ENC;
    op.op_ = enc;
    op.name_ = name;
    return op;
  }

  std::ostream& print(std::ostream& out) const {
    if (isReg()) out << "<reg> " << name_ << std::get<Register>(op_);
    else if (isImm()) out << "<imm> " << name_<< std::get<addr_t>(op_);
    else if (isEnc()) out << "<enc> " << name_ << std::get<addr_t>(op_);
    else if (!isValid()) out << "<invalid>" << name_;
    else out << "error_type";

    return out;
  }
};

class IInsn {
public:
  virtual const Operand& getOperand(uint8_t i) const = 0;
  virtual void addOperand(Operand op) = 0;
  virtual unsigned nOperands() const = 0;

  virtual addr_t getCode() const = 0;
  virtual void setCode(addr_t code) = 0;

  virtual addr_t getOpcode() const = 0;
  virtual RVInsnType getType() const = 0;

  virtual void execute(IRVModel& model) const = 0;

  virtual void print(std::ostream& out) const = 0;

  virtual ~IInsn() = default;
};

class RVInsn : public IInsn {
protected:
  std::vector<Operand> operands_; //< parts of insn encoding (not just operands)
  addr_t code_; //< full encoded insn
  addr_t opcode_; //< unique code used to determine operation
  RVInsnType type_ = RVInsnType::UNDEF_TYPE_INSN; //< instruction type

public:
  RVInsn(addr_t code, RVInsnType type = RVInsnType::UNDEF_TYPE_INSN) :
    code_(code), opcode_(code_ & DEFAULT_OPCODE_MASK), type_(type) {}

  // todo error checks
  const Operand& getOperand(uint8_t i) const override { return operands_[i]; }
  void addOperand(Operand op) override { operands_.push_back(op); }
  unsigned nOperands() const override { return operands_.size(); }

  addr_t getCode() const override { return code_; };
  void setCode(addr_t code) override { code_ = code; }

  addr_t getOpcode() const override { return opcode_; }
  RVInsnType getType() const override { return type_; }

  void print(std::ostream& out) const override {
    out << std::bitset<sizeof(addr_t) * BITS_BYTE>(getCode());
  }

  virtual ~RVInsn() = default;

  static std::unique_ptr<RVInsn> decode(addr_t code);
};

std::ostream& operator<< (std::ostream& out, const RVInsn& insn) {
  insn.print(out);
  return out;
}

std::ostream& operator<< (std::ostream& out, const Operand& op) {
  op.print(out);
  return out;
}

class GeneralUndefInsn final : public RVInsn {
public:
  GeneralUndefInsn(addr_t code) : RVInsn{code, RVInsnType::UNDEF_TYPE_INSN} {}

  void execute(IRVModel& model) const override;

  void print(std::ostream& out) const override {
    out << std::bitset<sizeof(addr_t) * BITS_BYTE>{getCode()} << " (?)";
  }
};

} // rv32i_sim

#endif // INSTRUCTION_HPP

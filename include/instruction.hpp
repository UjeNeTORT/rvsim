#ifndef INSTRUCTION_HPP
#define INSTRUCTION_HPP

#include <cassert>
#include <variant>

#include "encoding.hpp"

namespace rv32i_sim {

class RVModel;

class Operand {
  enum class OpType : uint8_t {
    INVALID = 0,
    REG = 1,
    IMM = 2,
  };

  std::variant<Register, uint64_t> op_;
  OpType type_ = OpType::INVALID;

public:
  bool isValid() const { return type_ != OpType::INVALID; }
  bool isReg() const { return type_ == OpType::REG; }
  bool isImm() const { return type_ == OpType::IMM; }

  Register getReg() const {
    assert(std::holds_alternative<Register>(op_) && "Variant holds another alternative");
    assert(isReg() && "Type mismatch");
    return std::get<Register>(op_);
  }

  uint64_t getImm() const {
    assert(std::holds_alternative<uint64_t>(op_) && "Variant holds another alternative");
    assert(isImm() && "Type mismatch");
    return std::get<uint64_t>(op_);
  }

  void setReg(Register reg) {
    assert(std::holds_alternative<Register>(op_) && "Variant holds another alternative");
    assert(isReg() && "Type mismatch");
    op_ = reg;
  }

  void setImm(uint64_t imm) {
    assert(std::holds_alternative<uint64_t>(op_) && "Variant holds another alternative");
    assert(isImm() && "Type mismatch");
    op_ = imm;
  }

  static Operand createReg(Register reg) {
    assert(isRegValid(reg) && "Invalid register");

    Operand op;
    op.type_ = OpType::REG;
    op.op_ = reg;
    return op;
  }

  static Operand createImm(uint64_t imm) {
    Operand op;
    op.type_ = OpType::IMM;
    op.op_ = imm;
    return op;
  }

  std::ostream& print(std::ostream& out) {
    if (isImm()) out << "imm: " << std::get<uint64_t>(op_);
    else if (isReg()) out << "reg: " << std::get<Register>(op_);
    else if (!isValid()) out << "invalid";
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

  virtual void execute(RVModel& model) const = 0;

  virtual ~IInsn() {};
};

class RVInsnNew : public IInsn {
protected:
  std::vector<Operand> operands_;
  addr_t code_; //< full encoded insn
  addr_t opcode_; //< unique code used to determine operation
  RVInsnType type_; //< instruction type (maybe should delete this field)

public:
  RVInsnNew(addr_t code) : code_(code), opcode_(code_ & DEFAULT_OPCODE_MASK) {}

  // todo error checks
  const Operand& getOperand(uint8_t i) const override { return operands_[i]; }
  void addOperand(Operand op) override { operands_.push_back(op); }
  unsigned nOperands() const override { return operands_.size(); }

  addr_t getCode() const override { return code_; };
  void setCode(addr_t code) override { code_ = code; }

  addr_t getOpcode() const override { return opcode_; }

  virtual ~RVInsnNew() = default;

  static RVInsnNew* decode(addr_t code);
};

std::ostream& operator<< (std::ostream& out, Operand op) {
  op.print(out);
  return out;
}

} // rv32i_sim

#endif // INSTRUCTION_HPP

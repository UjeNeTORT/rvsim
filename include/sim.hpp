#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include <array>
#include <cstdint>
#include <iostream>
#include <vector>

#include "encoding.hpp"
#include "memory.hpp"
#include "register_file.hpp"

namespace rv32i_sim {

class RVModel final {
  MemoryModel mem_;
  RegisterFile regs_;
  addr_t pc_;
  RVInsn insn_ = RVInsn{}; // current fetched insctruction

public:
  RVModel(addr_t pc_init = 0) : pc_(pc_init) {}
  RVModel(const MemoryModel& mem_init, const RegisterFile& regs_init, addr_t pc_init)
    : mem_(mem_init), regs_(regs_init), pc_(pc_init) {}

  RVModel(MemoryModel&& mem_init, RegisterFile&& regs_init, addr_t pc_init)
    : mem_(mem_init), regs_(regs_init), pc_(pc_init) {}

  void init(MemoryModel&& mem_init, RegisterFile&& regs_init, addr_t pc_init) {

  }

  void setPC(addr_t pc_new) {
    pc_ = pc_new;
  }

private:
  void decode() {
    addr_t instr_code = mem_.readWord(pc_); // insn fetch
    insn_ = RVInsn{instr_code}; // insn decode
  }

  void insn_execute() {
    switch (insn_.type())
    {
    case RVInsnType::R_TYPE_INSN:
    {
      Register dst = insn_.rd();
      word_t op1 = regs_.get(insn_.rs1());
      word_t op2 = regs_.get(insn_.rs2());


      switch (static_cast<RV32i_ISA>(insn_.code_opless()))
      {
      case RV32i_ISA::ADD:
        regs_.set(dst, op1 + op2);
        break;

      case RV32i_ISA::SUB:
        regs_.set(dst, op1 - op2);
        break;

      case RV32i_ISA::SLL:
        // performs logical left shift on the value in register rs1
        // by the shift amount held in the lower 5 bits of register rs2.
        regs_.set(dst, op1 << (op2 & 0x1F));
        break;

      case RV32i_ISA::SLT:
        // place the value 1 in register rd if register rs1
        // is less than register rs2 when both are treated
        // as signed numbers, else 0 is written to rd.
        regs_.set(dst, std::bit_cast<sword_t>(op1) < std::bit_cast<sword_t>(op2));
        break;

      case RV32i_ISA::SLTU:
        // place the value 1 in register rd if register rs1
        // is less than register rs2 when both are treated
        // as unsigned numbers, else 0 is written to rd.
        regs_.set(dst, op1 < op2);
        break;

      case RV32i_ISA::XOR:
        regs_.set(dst, op1 ^ op2);
        break;

      case RV32i_ISA::SRL:
        // logical right shift on the value in register rs1
        // by the shift amount held in the lower 5 bits of register rs2
        regs_.set(dst, op1 >> (op2 & 0x1F));
        break;

      case RV32i_ISA::SRA:
        // performs arithmetic right shift on the value
        // in register rs1 by the shift amount held
        // in the lower 5 bits of register rs2
        regs_.set(dst, std::bit_cast<sword_t>(op1) >> (op2 & 0x1F));
        break;

      case RV32i_ISA::OR:
        regs_.set(dst, op1 | op2);
        break;

      case RV32i_ISA::AND:
        regs_.set(dst, op1 & op2);
        break;

      default:
        std::cerr << "R type invalid insn\n";
        break;
      }
    }
      break;
    // todo
    case RVInsnType::I_TYPE_INSN:
    case RVInsnType::S_TYPE_INSN:
    case RVInsnType::U_TYPE_INSN:
    case RVInsnType::UNDEF_TYPE_INSN:
    default:
      break;
    }

  }

public:
  void execute() {

    while(true) {
      decode();
      if (insn_.type() == RVInsnType::UNDEF_TYPE_INSN) break;

      insn_execute();
      setPC(pc_ + sizeof(word_t));
    }

  }

  std::ostream& dump(std::ostream& out) {
    out << "pc = " << pc_ << '\n';
    regs_.dump(out);
    addr_t start_addr = 0;
    addr_t end_addr = (pc_ > ADDR_SPACE_CAPACITY - 4) ? ADDR_SPACE_CAPACITY : pc_ + 4;
    mem_.dump(out, start_addr, end_addr); // todo fix magic numbers
    return out;
  }

  std::ostream& dump_all(std::ostream& out) {
    out << "pc = " << pc_ << '\n';
    regs_.dump(out);
    mem_.dump(out);
    return out;
  }
};

std::ostream& operator<<(std::ostream& out, RVModel& model) {
  model.dump(out);
  return out;
}

} // namespace rv32i_sim

#endif // SIMULATOR_HPP

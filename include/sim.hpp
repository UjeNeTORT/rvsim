#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include <array>
#include <cstdint>
#include <iostream>
#include <vector>

#include "encoding.hpp"
#include "memory.hpp"
#include "registers.hpp"

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

  ~RVModel() {}

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
    case R_TYPE_INSN:
      switch (insn_.code_opless())
      {
      case RV32i_ISA::ADD:
        break;
      case RV32i_ISA::SUB:
        break;
      case RV32i_ISA::SLL:
        break;
      case RV32i_ISA::SLT:
        break;
      case RV32i_ISA::SLTU:
        break;
      case RV32i_ISA::XOR:
        break;
      case RV32i_ISA::SRL:
        break;
      case RV32i_ISA::SRA:
        break;
      case RV32i_ISA::OR:
        break;
      case RV32i_ISA::AND:
        break;
      default:
        break;
      }

      break;
    // todo
    case I_TYPE_INSN:
    case S_TYPE_INSN:
    case U_TYPE_INSN:
    case UNDEF_TYPE_INSN:
    default:
      break;
    }

  }

public:
  void execute() {

  }
};

} // namespace rv32i_sim

#endif // SIMULATOR_HPP

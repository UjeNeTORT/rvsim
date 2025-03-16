#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include <array>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "encoding.hpp"
#include "memory.hpp"
#include "register_file.hpp"

namespace rv32i_sim {

int32_t sign_extend_8_to_32(uint8_t val) {
  return static_cast<int32_t>(uint32_t(val) << 24) >> 24;
}

int32_t sign_extend_12_to_32(uint16_t val) {
  return static_cast<int32_t>(uint32_t(val) << 20) >> 20;
}

int32_t sign_extend_16_to_32(uint16_t val) {
  return static_cast<int32_t>(uint32_t(val) << 16) >> 16;
}

int32_t sign_extend_32_to_32(uint32_t val) {
  return std::bit_cast<int32_t>(val);
}

const std::string RV32I_MODEL_STATE_SIGNATURE = "RV32I_MDL_STATE";

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

  void init(std::ifstream& model_state_file) {
    if (!model_state_file) {
      std::cerr << "ERROR: wrong model state file\n";
      return;
    }

    std::string signature(RV32I_MODEL_STATE_SIGNATURE.size(), ' ');
    model_state_file.read(signature.data(), RV32I_MODEL_STATE_SIGNATURE.size() + 1);
    if (signature != RV32I_MODEL_STATE_SIGNATURE) {
      std::cerr << "ERROR: model state file signature mismatch:\n"
                << "      <" << signature << "> vs <" << RV32I_MODEL_STATE_SIGNATURE <<">\n";
      return;
    }

    model_state_file.read(reinterpret_cast<char *>(&pc_), sizeof(addr_t));
    regs_ = RegisterFile{model_state_file};
    mem_ = MemoryModel{model_state_file};
  }

  void init(const MemoryModel& mem_init, const RegisterFile& regs_init, addr_t pc_init) {
    mem_ = mem_init; regs_ = regs_init; pc_ = pc_init;
  }

  void init(MemoryModel&& mem_init, RegisterFile&& regs_init, addr_t pc_init) {
    mem_ = mem_init; regs_ = regs_init; pc_ = pc_init;
  }

  RVModel(std::ifstream& model_state_file) { init(model_state_file); }

  void setPC(addr_t pc_new) { pc_ = pc_new; }

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

    case RVInsnType::I_TYPE_INSN:
    {
      Register dst = insn_.rd();
      word_t op1 = regs_.get(insn_.rs1());
      uint16_t imm = insn_.imm11_0();

      switch (static_cast<RV32i_ISA>(insn_.code_opless()))
      {
      // Jump to an address formed by adding rs1 to
      // a signed offset then clearing the least significant bit,
      // and store the return address in rd.
      case RV32i_ISA::JALR:
        regs_.set(dst, pc_ + sizeof(addr_t));
        setPC(op1 + sign_extend_12_to_32(imm));
        break;

      case RV32i_ISA::LB:
        regs_.set(dst, sign_extend_8_to_32(mem_.readByte(op1 + sign_extend_12_to_32(imm))));
        break;

      case RV32i_ISA::LH:
        regs_.set(dst, sign_extend_16_to_32(mem_.readHalf(op1 + sign_extend_12_to_32(imm))));
        break;

      case RV32i_ISA::LW:
        regs_.set(dst, sign_extend_32_to_32(mem_.readWord(op1 + sign_extend_12_to_32(imm))));
        break;

      case RV32i_ISA::LBU:
        regs_.set(dst, mem_.readByte(op1 + sign_extend_12_to_32(imm)));
        break;

      case RV32i_ISA::LHU:
        regs_.set(dst, mem_.readHalf(op1 + sign_extend_12_to_32(imm)));
        break;

      case RV32i_ISA::ADDI:
        regs_.set(dst, op1 + sign_extend_12_to_32(imm));
        break;

      case RV32i_ISA::SLTI:
        regs_.set(dst, sign_extend_32_to_32(op1) < sign_extend_12_to_32(imm));
        break;

      case RV32i_ISA::SLTIU:
        regs_.set(dst, op1 < std::bit_cast<uint32_t>(sign_extend_12_to_32(imm)));
        break;

      case RV32i_ISA::XORI:
        regs_.set(dst, op1 ^ sign_extend_12_to_32(imm));
        break;

      case RV32i_ISA::ORI:
        regs_.set(dst, op1 | sign_extend_12_to_32(imm));
        break;

      case RV32i_ISA::ANDI:
        regs_.set(dst, op1 & sign_extend_12_to_32(imm));
        break;

      default:
        break;
      }
    }
      break;
    // todo
    case RVInsnType::S_TYPE_INSN:
    case RVInsnType::U_TYPE_INSN:
    case RVInsnType::UNDEF_TYPE_INSN:
    default:
      break;
    }
  }

public:
  void execute() {
    std::cerr << "DBG: begin execution (pc = " << pc_ << ")\n";
    while(true) {
      decode();
      std::cerr << insn_ << '\n';
      if (insn_.type() == RVInsnType::UNDEF_TYPE_INSN) break; // todo this is shit
      insn_execute();
      setPC(pc_ + sizeof(word_t));
    }
    std::cerr << "DBG: end execution (pc = " << pc_ << ")\n";
  }

  std::ostream& dump(std::ostream& out) {
    out << "pc = " << pc_ << '\n';
    regs_.dump(out);
    mem_.dump(out);
    return out;
  }

  void binary_dump(std::ofstream& fout) {
    if (!fout) {
      std::cerr << "ERROR: wrong fout\n";
      return;
    }

    fout.write(RV32I_MODEL_STATE_SIGNATURE.c_str(),
               RV32I_MODEL_STATE_SIGNATURE.size() + 1);
    fout.write(reinterpret_cast<char *>(&pc_), sizeof(addr_t));
    regs_.binary_dump(fout);
    mem_.binary_dump(fout);
  }
};

std::ostream& operator<<(std::ostream& out, RVModel& model) {
  model.dump(out);
  return out;
}

} // namespace rv32i_sim

#endif // SIMULATOR_HPP

#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include <array>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "isa.hpp"
#include "instruction.hpp"
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
  IInsn *insn_ = nullptr; // current fetched insctruction

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
    insn_ = RVInsnNew::decode(instr_code);
  }

  void insn_execute() {
    insn_->execute(*this);
  }

public:
  void execute() {
    std::cerr << "DBG: begin execution (pc = " << pc_ << ")\n";
    while(true) {
      decode();
      insn_execute();
      delete insn_;
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

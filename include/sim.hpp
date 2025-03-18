#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include <array>
#include <bit>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "isim.hpp"
#include "instruction.hpp"
#include "isa.hpp"
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

class RVModel final : IRVModel {
  MemoryModel mem_;
  RegisterFile regs_;
  addr_t pc_;

public:
  RVModel(addr_t pc_init = 0) : pc_(pc_init) {}
  RVModel(const MemoryModel& mem_init, const RegisterFile& regs_init, addr_t pc_init)
    : mem_(mem_init), regs_(regs_init), pc_(pc_init) {}

  RVModel(MemoryModel&& mem_init, RegisterFile&& regs_init, addr_t pc_init)
    : mem_(mem_init), regs_(regs_init), pc_(pc_init) {}

  void init(std::ifstream& model_state_file) override;
  void init(const MemoryModel& mem_init, const RegisterFile& regs_init,
                                                              addr_t pc_init) override;
  void init(MemoryModel&& mem_init, RegisterFile&& regs_init, addr_t pc_init) override;

  RVModel(std::ifstream& model_state_file) { init(model_state_file); }

  addr_t getPC() const override;
  void setPC(addr_t pc_new) override;

private:
  IInsn* decode();

public:
  byte_t readByte(addr_t addr) override;
  half_t readHalf(addr_t addr) override;
  word_t readWord(addr_t addr) override;

#ifdef RVBITS64
  dword_t readDWord(addr_t addr) override;
#endif // RVBITS64

  void writeByte(addr_t addr, byte_t val) override;
  void writeHalf(addr_t addr, half_t val) override;
  void writeWord(addr_t addr, word_t val) override;

#ifdef RVBITS64
  void writeDWord(addr_t addr, dword_t val) override;
#endif // RVBITS64

  addr_t getReg(Register reg) const override;
  void setReg(Register reg, word_t val) override;

  void execute() override;

  std::ostream& print(std::ostream& out) override;
  void binaryDump(std::ofstream& fout) override;
};

void RVModel::init(std::ifstream& model_state_file) {
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

void RVModel::init(const MemoryModel& mem_init, const RegisterFile& regs_init, addr_t pc_init) {
  mem_ = mem_init; regs_ = regs_init; pc_ = pc_init;
}

void RVModel::init(MemoryModel&& mem_init, RegisterFile&& regs_init, addr_t pc_init) {
  mem_ = mem_init; regs_ = regs_init; pc_ = pc_init;
}

addr_t RVModel::getPC() const {
  return pc_;
}

void RVModel::setPC(addr_t pc_new) {
  pc_ = pc_new;
}

byte_t RVModel::readByte(addr_t addr) { return mem_.readByte(addr); }
half_t RVModel::readHalf(addr_t addr) { return mem_.readHalf(addr); }
word_t RVModel::readWord(addr_t addr) { return mem_.readWord(addr); }

#ifdef RVBITS64
dword_t RVModel::readDWord(addr_t addr) { return mem_.readDword(addr); }
#endif // RVBITS64

void RVModel::writeByte(addr_t addr, byte_t val) { mem_.writeByte(addr, val); }
void RVModel::writeHalf(addr_t addr, half_t val) { mem_.writeHalf(addr, val); }
void RVModel::writeWord(addr_t addr, word_t val) { mem_.writeWord(addr, val); }

#ifdef RVBITS64
void RVModel::writeDWord(addr_t addr, dword_t val) { mem_.writeDword(addr, val); }
#endif // RVBITS64

IInsn* RVModel::decode() {
  addr_t instr_code = mem_.readWord(pc_); // insn fetch
  return RVInsn::decode(instr_code);
}

void RVModel::execute() {
  std::cerr << "DBG: begin execution (pc = " << pc_ << ")\n";

  while(true) {
    IInsn* insn = decode();
    insn->execute(*this);

    if (insn->getType() == RVInsnType::UNDEF_TYPE_INSN) {
      delete insn;
      break; // temporary
    }
    delete insn;

    setPC(pc_ + sizeof(word_t));
  }
  std::cerr << "DBG: end execution (pc = " << pc_ << ")\n";
}

std::ostream& RVModel::print(std::ostream& out) {
  out << "pc = " << getPC() << '\n';
  regs_.print(out);
  mem_.print(out);
  return out;
}

void RVModel::binaryDump(std::ofstream& fout) {
  if (!fout) {
    std::cerr << "ERROR: wrong fout\n";
    return;
  }

  fout.write(RV32I_MODEL_STATE_SIGNATURE.c_str(),
             RV32I_MODEL_STATE_SIGNATURE.size() + 1);
  fout.write(reinterpret_cast<char *>(&pc_), sizeof(addr_t));
  regs_.binaryDump(fout);
  mem_.binaryDump(fout);
}

addr_t RVModel::getReg(Register reg) const {
  return regs_.get(reg);
}

void RVModel::setReg(Register reg, word_t val) {
  regs_.set(reg, val);
}

void rvADD::execute(IRVModel& model) const {
  Register dst = getOperand(1).getReg();
  Register rs1 = getOperand(3).getReg();
  Register rs2 = getOperand(4).getReg();

  addr_t op1 = model.getReg(rs1);
  addr_t op2 = model.getReg(rs2);
  model.setReg(dst, op1 + op2);

  std::cerr << *this << " add <pc = " << model.getPC() << ">\n";
}

void rvSUB::execute(IRVModel& model) const {
  Register dst = getOperand(1).getReg();
  Register rs1 = getOperand(3).getReg();
  Register rs2 = getOperand(4).getReg();

  addr_t op1 = model.getReg(rs1);
  addr_t op2 = model.getReg(rs2);
  model.setReg(dst, op1 - op2);

  std::cerr << *this << " sub <pc = " << model.getPC() << ">\n";
}

void rvSLL::execute(IRVModel& model) const {
  Register dst = getOperand(1).getReg();
  Register rs1 = getOperand(3).getReg();
  Register rs2 = getOperand(4).getReg();

  addr_t op1 = model.getReg(rs1);
  addr_t op2 = model.getReg(rs2);
  model.setReg(dst, op1 << (op2 & MASK_4_0));

  std::cerr << *this << " sll <pc = " << model.getPC() << ">\n";
}

void rvSLT::execute(IRVModel& model) const {
  Register dst = getOperand(1).getReg();
  Register rs1 = getOperand(3).getReg();
  Register rs2 = getOperand(4).getReg();

  addr_t op1 = model.getReg(rs1);
  addr_t op2 = model.getReg(rs2);
  model.setReg(dst, std::bit_cast<sword_t>(op1) < std::bit_cast<sword_t>(op2));
  std::cerr << *this << " slt <pc = " << model.getPC() << ">\n";
}

void rvSLTU::execute(IRVModel& model) const {
  Register dst = getOperand(1).getReg();
  Register rs1 = getOperand(3).getReg();
  Register rs2 = getOperand(4).getReg();

  addr_t op1 = model.getReg(rs1);
  addr_t op2 = model.getReg(rs2);
  model.setReg(dst, op1 < op2);

  std::cerr << *this << " sltu <pc = " << model.getPC() << ">\n";
}

void rvXOR::execute(IRVModel& model) const {
  Register dst = getOperand(1).getReg();
  Register rs1 = getOperand(3).getReg();
  Register rs2 = getOperand(4).getReg();

  addr_t op1 = model.getReg(rs1);
  addr_t op2 = model.getReg(rs2);
  model.setReg(dst, op1 ^ op2);

  std::cerr << *this << " xor <pc = " << model.getPC() << ">\n";
}

void rvSRL::execute(IRVModel& model) const {
  Register dst = getOperand(1).getReg();
  Register rs1 = getOperand(3).getReg();
  Register rs2 = getOperand(4).getReg();

  addr_t op1 = model.getReg(rs1);
  addr_t op2 = model.getReg(rs2);
  model.setReg(dst, op1 >> (op2 & MASK_4_0));

  std::cerr << *this << " srl <pc = " << model.getPC() << ">\n";
}

void rvSRA::execute(IRVModel& model) const {
  Register dst = getOperand(1).getReg();
  Register rs1 = getOperand(3).getReg();
  Register rs2 = getOperand(4).getReg();

  addr_t op1 = model.getReg(rs1);
  addr_t op2 = model.getReg(rs2);
  model.setReg(dst, std::bit_cast<sword_t>(op1) >> (op2 & MASK_4_0));

  std::cerr << *this << " sra <pc = " << model.getPC() << ">\n";
}

void rvOR::execute(IRVModel& model) const {
  Register dst = getOperand(1).getReg();
  Register rs1 = getOperand(3).getReg();
  Register rs2 = getOperand(4).getReg();

  addr_t op1 = model.getReg(rs1);
  addr_t op2 = model.getReg(rs2);
  model.setReg(dst, op1 | op2);
  std::cerr << *this << " or <pc = " << model.getPC() << ">\n";
}

void rvAND::execute(IRVModel& model) const {
  Register dst = getOperand(1).getReg();
  Register rs1 = getOperand(3).getReg();
  Register rs2 = getOperand(4).getReg();

  addr_t op1 = model.getReg(rs1);
  addr_t op2 = model.getReg(rs2);
  model.setReg(dst, op1 & op2);
  std::cerr << *this << " and <pc = " << model.getPC() << ">\n";
}

void rvUNDEF::execute(IRVModel& model) const {
  std::cerr << *this << " ??? <pc = " << model.getPC() << ">\n";
}

void GeneralUndefInsn::execute(IRVModel& model) const {
  std::cerr << *this << " ??? <pc = " << model.getPC() << ">\n";
}

} // namespace rv32i_sim

#endif // SIMULATOR_HPP

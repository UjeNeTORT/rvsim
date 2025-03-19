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

int32_t sign_extend_8_to_32(uint8_t val) {
  return std::bit_cast<int32_t>(uint32_t(val) << 24) >> 24;
}

int32_t sign_extend_12_to_32(uint16_t val) {
  return std::bit_cast<int32_t>(uint32_t(val) << 20) >> 20;
}

int32_t sign_extend_13_to_32(uint16_t val) {
  return std::bit_cast<int32_t>(uint32_t(val) << 19) >> 19;
}

int32_t sign_extend_16_to_32(uint16_t val) {
  return std::bit_cast<int32_t>(uint32_t(val) << 16) >> 16;
}

int32_t sign_extend_32_to_32(uint32_t val) {
  return std::bit_cast<int32_t>(val);
}

namespace rv32i_sim {

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
  std::unique_ptr<IInsn> decode(addr_t insn_code);

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
              << "      <" << signature << "> vs <"
                                            << RV32I_MODEL_STATE_SIGNATURE <<">\n";
    return;
  }

  model_state_file.read(reinterpret_cast<char *>(&pc_), sizeof(addr_t));
  regs_ = RegisterFile{model_state_file};
  mem_ = MemoryModel{model_state_file};

  assert(pc_ % IALIGN == 0 && "PC at unaligned position");
}

void RVModel::init(const MemoryModel& mem_init, const RegisterFile& regs_init, addr_t pc_init) {
  mem_ = mem_init; regs_ = regs_init; pc_ = pc_init;
  assert(pc_ % IALIGN == 0 && "PC at unaligned position");
}

void RVModel::init(MemoryModel&& mem_init, RegisterFile&& regs_init, addr_t pc_init) {
  mem_ = mem_init; regs_ = regs_init; pc_ = pc_init;
  assert(pc_ % IALIGN == 0 && "PC at unaligned position");
}

addr_t RVModel::getPC() const {
  return pc_;
}

void RVModel::setPC(addr_t pc_new) {
  assert(pc_new % IALIGN == 0 && "PC set to unaligned position");
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

std::unique_ptr<IInsn> RVModel::decode(addr_t insn_code) {
  return RVInsn::decode(insn_code);
}

void RVModel::execute() {
  std::cerr << "DBG: begin execution (pc = " << pc_ << ")\n";

  while(true) {
    addr_t insn_code = mem_.readWord(pc_); // insn fetch
    std::unique_ptr<IInsn> insn = decode(insn_code);
    insn->execute(*this);

    if (insn->getType() == RVInsnType::UNDEF_TYPE_INSN) break; // temporary measure

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
  addr_t op1 = model.getReg(rs1_);
  addr_t op2 = model.getReg(rs2_);
  model.setReg(dst_, op1 + op2);

  std::cerr << *this << " add <pc = " << model.getPC() << ">\n";
}

void rvSUB::execute(IRVModel& model) const {
  addr_t op1 = model.getReg(rs1_);
  addr_t op2 = model.getReg(rs2_);
  model.setReg(dst_, op1 - op2);

  std::cerr << *this << " sub <pc = " << model.getPC() << ">\n";
}

void rvSLL::execute(IRVModel& model) const {
  addr_t op1 = model.getReg(rs1_);
  addr_t op2 = model.getReg(rs2_);
  model.setReg(dst_, op1 << (op2 & MASK_4_0));

  std::cerr << *this << " sll <pc = " << model.getPC() << ">\n";
}

void rvSLT::execute(IRVModel& model) const {
  addr_t op1 = model.getReg(rs1_);
  addr_t op2 = model.getReg(rs2_);
  model.setReg(dst_, std::bit_cast<sword_t>(op1) < std::bit_cast<sword_t>(op2));

  std::cerr << *this << " slt <pc = " << model.getPC() << ">\n";
}

void rvSLTU::execute(IRVModel& model) const {
  addr_t op1 = model.getReg(rs1_);
  addr_t op2 = model.getReg(rs2_);
  model.setReg(dst_, op1 < op2);

  std::cerr << *this << " sltu <pc = " << model.getPC() << ">\n";
}

void rvXOR::execute(IRVModel& model) const {
  addr_t op1 = model.getReg(rs1_);
  addr_t op2 = model.getReg(rs2_);
  model.setReg(dst_, op1 ^ op2);

  std::cerr << *this << " xor <pc = " << model.getPC() << ">\n";
}

void rvSRL::execute(IRVModel& model) const {
  addr_t op1 = model.getReg(rs1_);
  addr_t op2 = model.getReg(rs2_);
  model.setReg(dst_, op1 >> (op2 & MASK_4_0));
}

void rvSRA::execute(IRVModel& model) const {
  addr_t op1 = model.getReg(rs1_);
  addr_t op2 = model.getReg(rs2_);
  model.setReg(dst_, std::bit_cast<sword_t>(op1) >> (op2 & MASK_4_0));

  std::cerr << *this << " sra <pc = " << model.getPC() << ">\n";
}

void rvOR::execute(IRVModel& model) const {
  addr_t op1 = model.getReg(rs1_);
  addr_t op2 = model.getReg(rs2_);
  model.setReg(dst_, op1 | op2);

  std::cerr << *this << " or <pc = " << model.getPC() << ">\n";
}

void rvAND::execute(IRVModel& model) const {
  addr_t op1 = model.getReg(rs1_);
  addr_t op2 = model.getReg(rs2_);
  model.setReg(dst_, op1 & op2);

  std::cerr << *this << " and <pc = " << model.getPC() << ">\n";
}

void rvUNDEF_R::execute(IRVModel& model) const {
  // do nothing
  std::cerr << *this << " ??? <pc = " << model.getPC() << ">\n";
}

void rvJALR::execute(IRVModel& model) const {
  addr_t ret_addr = model.getPC() + sizeof(word_t);
  model.setReg(dst_, ret_addr);

  addr_t jmp_addr = model.getReg(rs1_) + sign_extend_12_to_32(imm_);
  jmp_addr &= 0xFFFF'FFFE; // clear least significant bit
  model.setPC(jmp_addr);

  std::cerr << *this << " jalr <pc = " << model.getPC() << ">\n";
}

void rvLB::execute(IRVModel& model) const {
  addr_t mem_addr = model.getReg(rs1_) + sign_extend_12_to_32(imm_);
  byte_t mem_val = model.readByte(mem_addr);
  model.setReg(dst_, sign_extend_8_to_32(mem_val));

  std::cerr << *this << " lb <pc = " << model.getPC() << ">\n";
}

void rvLH::execute(IRVModel& model) const {
  addr_t mem_addr = model.getReg(rs1_) + sign_extend_12_to_32(imm_);
  half_t mem_val = model.readHalf(mem_addr);
  model.setReg(dst_, sign_extend_16_to_32(mem_val));

  std::cerr << *this << " lh <pc = " << model.getPC() << ">\n";
}

void rvLW::execute(IRVModel& model) const {
  addr_t mem_addr = model.getReg(rs1_) + sign_extend_12_to_32(imm_);
  word_t mem_val = model.readWord(mem_addr);
  model.setReg(dst_, sign_extend_16_to_32(mem_val));

  std::cerr << *this << " lw <pc = " << model.getPC() << ">\n";
}

void rvLBU::execute(IRVModel& model) const {
  addr_t mem_addr = model.getReg(rs1_) + sign_extend_12_to_32(imm_);
  byte_t mem_val = model.readByte(mem_addr);
  model.setReg(dst_, static_cast<addr_t>(mem_val));

  std::cerr << *this << " lbu <pc = " << model.getPC() << ">\n";
}

void rvLHU::execute(IRVModel& model) const {
  addr_t mem_addr = model.getReg(rs1_) + sign_extend_12_to_32(imm_);
  half_t mem_val = model.readHalf(mem_addr);
  model.setReg(dst_, static_cast<addr_t>(mem_val));

  std::cerr << *this << " lhu <pc = " << model.getPC() << ">\n";
}

void rvADDI::execute(IRVModel& model) const {
  addr_t op1 = model.getReg(rs1_);
  model.setReg(dst_, op1 + sign_extend_12_to_32(imm_));

  std::cerr << *this << " addi <pc = " << model.getPC() << ">\n";
}

void rvSLTI::execute(IRVModel& model) const {
  addr_t op1 = model.getReg(rs1_);
  model.setReg(dst_, std::bit_cast<sword_t>(op1) < sign_extend_12_to_32(imm_));

  std::cerr << *this << " slti <pc = " << model.getPC() << ">\n";
}

void rvSLTIU::execute(IRVModel& model) const {
  addr_t op1 = model.getReg(rs1_);
  model.setReg(dst_, op1 < std::bit_cast<addr_t>(sign_extend_12_to_32(imm_)));

  std::cerr << *this << " sltiu <pc = " << model.getPC() << ">\n";
}

void rvXORI::execute(IRVModel& model) const {
  sword_t signed_imm = sign_extend_12_to_32(imm_);
  addr_t op1 = model.getReg(rs1_);

  // Note, “XORI rd, rs1, -1” performs a bitwise logical
  // inversion of register rs1
  // (assembler pseudo-instruction NOT rd, rs)
  //
  // source: https://msyksphinz-self.github.io/riscv-isadoc/html/rvi.html#lb
  if (signed_imm == -1)
    model.setReg(dst_, ~op1);
  else
    model.setReg(dst_, op1 ^ signed_imm);

  std::cerr << *this << " xori <pc = " << model.getPC() << ">\n";
}

void rvORI::execute(IRVModel& model) const {
  addr_t op1 = model.getReg(rs1_);
  model.setReg(dst_, op1 | sign_extend_12_to_32(imm_));

  std::cerr << *this << " ori <pc = " << model.getPC() << ">\n";
}

void rvANDI::execute(IRVModel& model) const {
  addr_t op1 = model.getReg(rs1_);
  model.setReg(dst_, op1 & sign_extend_12_to_32(imm_));

  std::cerr << *this << " andi <pc = " << model.getPC() << ">\n";
}

void rvSLLI::execute(IRVModel& model) const {
  addr_t op1 = model.getReg(rs1_);
  addr_t shamt = imm_ & MASK_4_0;

  model.setReg(dst_, op1 << shamt);

  std::cerr << *this << " slli <pc = " << model.getPC() << ">\n";
}

void rvSRLI::execute(IRVModel& model) const {
  addr_t op1 = model.getReg(rs1_);
  addr_t shamt = imm_ & MASK_4_0;

  model.setReg(dst_, op1 >> shamt);

  std::cerr << *this << " srli <pc = " << model.getPC() << ">\n";
}

void rvSRAI::execute(IRVModel& model) const {
  addr_t op1 = model.getReg(rs1_);
  addr_t shamt = imm_ & MASK_4_0;

  model.setReg(dst_, std::bit_cast<sword_t>(op1) >> shamt);

  std::cerr << *this << " srai <pc = " << model.getPC() << ">\n";
}

void rvUNDEF_I::execute(IRVModel& model) const {
  // do nothing
  std::cerr << *this << " ??? <pc = " << model.getPC() << ">\n";
}

void rvSB::execute(IRVModel& model) const {
  addr_t op1 = model.getReg(rs1_);
  addr_t op2 = model.getReg(rs2_);
  addr_t mem_addr = op1 + sign_extend_12_to_32(imm_);
  byte_t val = op2 & 0xFF; // 8 bits mask

  model.writeByte(mem_addr, val);

  std::cerr << *this << "  sb <pc = " << model.getPC() << ">\n";
};

void rvSH::execute(IRVModel& model) const {
  addr_t op1 = model.getReg(rs1_);
  addr_t op2 = model.getReg(rs2_);
  addr_t mem_addr = op1 + sign_extend_12_to_32(imm_);
  half_t val = op2 & 0xFFFF; // 16 bits mask

  model.writeHalf(mem_addr, val);

  std::cerr << *this << "  sh <pc = " << model.getPC() << ">\n";
};

void rvSW::execute(IRVModel& model) const {
  addr_t op1 = model.getReg(rs1_);
  addr_t op2 = model.getReg(rs2_);
  addr_t mem_addr = op1 + sign_extend_12_to_32(imm_);
  word_t val = op2 & 0xFFFF'FFFF; // 32 bits mask

  model.writeWord(mem_addr, val);

  std::cerr << *this << "  sw <pc = " << model.getPC() << ">\n";
};

void rvUNDEF_S::execute(IRVModel& model) const {
  // do nothing
  std::cerr << *this << " ??? <pc = " << model.getPC() << ">\n";
}

void rvBEQ::execute(IRVModel& model) const {
  addr_t op1 = model.getReg(rs1_);
  addr_t op2 = model.getReg(rs2_);

  if (op1 == op2) {
    addr_t branch_addr = model.getPC() + sign_extend_13_to_32(imm_);
    model.setPC(branch_addr);
  }

  std::cerr << *this << " beq <pc =" << model.getPC() << ">\n";
}

void rvBNE::execute(IRVModel& model) const {
  addr_t op1 = model.getReg(rs1_);
  addr_t op2 = model.getReg(rs2_);

  if (op1 != op2) {
    addr_t branch_addr = model.getPC() + sign_extend_13_to_32(imm_);
    model.setPC(branch_addr);
  }

  std::cerr << *this << " bne <pc =" << model.getPC() << ">\n";
}

void rvBLT::execute(IRVModel& model) const {
  sword_t op1 = std::bit_cast<sword_t>(model.getReg(rs1_));
  sword_t op2 = std::bit_cast<sword_t>(model.getReg(rs2_));

  if (op1 < op2) {
    addr_t branch_addr = model.getPC() + sign_extend_13_to_32(imm_);
    model.setPC(branch_addr);
  }

  std::cerr << *this << " blt <pc =" << model.getPC() << ">\n";
}

void rvBLTU::execute(IRVModel& model) const {
  addr_t op1 = model.getReg(rs1_);
  addr_t op2 = model.getReg(rs2_);

  if (op1 < op2) {
    addr_t branch_addr = model.getPC() + sign_extend_13_to_32(imm_);
    model.setPC(branch_addr);
  }

  std::cerr << *this << " bltu <pc =" << model.getPC() << ">\n";
}

void rvBGE::execute(IRVModel& model) const {
  sword_t op1 = std::bit_cast<sword_t>(model.getReg(rs1_));
  sword_t op2 = std::bit_cast<sword_t>(model.getReg(rs2_));

  if (op1 >= op2) {
    addr_t branch_addr = model.getPC() + sign_extend_13_to_32(imm_);
    model.setPC(branch_addr);
  }

  std::cerr << *this << " bge <pc =" << model.getPC() << ">\n";
}

void rvBGEU::execute(IRVModel& model) const {
  addr_t op1 = model.getReg(rs1_);
  addr_t op2 = model.getReg(rs2_);

  if (op1 >= op2) {
    addr_t branch_addr = model.getPC() + sign_extend_13_to_32(imm_);
    model.setPC(branch_addr);
  }

  std::cerr << *this << " bgeu <pc =" << model.getPC() << ">\n";
}

void rvUNDEF_B::execute(IRVModel& model) const {
  // do nothing
  std::cerr << *this << " ??? <pc =" << model.getPC() << ">\n";
}

void GeneralUndefInsn::execute(IRVModel& model) const {
  // do nothing
  std::cerr << *this << " ??? <pc = " << model.getPC() << ">\n";
}

} // namespace rv32i_sim

#endif // SIMULATOR_HPP

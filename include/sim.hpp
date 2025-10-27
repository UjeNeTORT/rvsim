#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include <array>
#include <bit>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include <elfio/elfio.hpp>

#include "isim.hpp"
#include "instruction.hpp"
#include "memory.hpp"
#include "register_file.hpp"
#include "exec_env.hpp"

#include "decoder_helpers.hpp"
#include "decoder.inc"

namespace elf = ELFIO;

namespace rv32i_sim {

const std::string RV32I_MODEL_STATE_SIGNATURE = "RV32I_MDL_STATE";

// todo refactor mess
class RVModel final : IRVModel {
  MemoryModel mem_;
  RegisterFile regs_;
  uint32_t pc_;

  bool execution = false;
  bool is_valid_ = false;

public:
  RVModel(uint32_t pc_init = 0) : pc_(pc_init) {}
  RVModel(const MemoryModel& mem_init, const RegisterFile& regs_init, uint32_t pc_init)
    : mem_(mem_init), regs_(regs_init), pc_(pc_init) {}

  RVModel(MemoryModel&& mem_init, RegisterFile&& regs_init, uint32_t pc_init)
    : mem_(mem_init), regs_(regs_init), pc_(pc_init) {}

  RVModel(std::filesystem::path& elf_path) {
    elf::elfio elf_reader;
    if (!elf_reader.load(elf_path)) {
      std::cerr << "ERROR: failed to load ELF " << elf_path << "\n";
      is_valid_ = false;
      return;
    }

    pc_ = elf_reader.get_entry();
    std::cerr << "Found user entry point at: " << pc_ << '\n';

    regs_ = RegisterFile();
    mem_ = MemoryModel::fromELF(elf_reader);

    // setting up stack and initial stack frame
    uint32_t sp = mem_.setUpStack();
    regs_.set(Register::X2, sp); // SP = sp
    regs_.set(Register::X8, sp); // FP = sp

    // preparing execution environment i.e.
    // code which calls main and does ebreak in the end
    pc_ = setUpEnvironment(pc_);

    is_valid_ = mem_.isValid() && regs_.isValid();
  }

  void init(std::ifstream& model_state_file) override;
  void init(std::filesystem::path& model_state_path) {
    std::ifstream bstate_file(model_state_path);
    if (!bstate_file) {
      std::cerr << "ERROR: failed to open bstate file " << model_state_path << "\n";
      is_valid_ = false;
      return;
    }

    init(bstate_file);
  }

  void init(const MemoryModel& mem_init, const RegisterFile& regs_init,
                                                              uint32_t pc_init) override;
  void init(MemoryModel&& mem_init, RegisterFile&& regs_init, uint32_t pc_init) override;

  bool operator== (const RVModel& other) const;

  uint32_t getPC() const override;
  void setPC(uint32_t pc_new) override;

private:
  std::unique_ptr<RVISA::IRVInsn> decode(uint32_t insn_code);
  void printInsn(std::ostream& out, const RVISA::IRVInsn& insn);

public:
  bool isValid() const override;

  uint8_t readByte(uint32_t addr) const override;
  uint16_t readHalf(uint32_t addr) const override;
  uint32_t readWord(uint32_t addr) const override;

  void writeByte(uint32_t addr, uint8_t val) override;
  void writeHalf(uint32_t addr, uint16_t val) override;
  void writeWord(uint32_t addr, uint32_t val) override;

  uint32_t getReg(Register reg) const override;
  void setReg(Register reg, uint32_t val) override;

  uint32_t setUpEnvironment(uint32_t pc_main);

  void execute() override;
  void exit() override;

  std::ostream& print(std::ostream& out) override;
  void binaryDump(std::ofstream& fout) override;
};

void RVModel::init(std::ifstream& model_state_file) {
  if (!model_state_file) {
    std::cerr << "ERROR: wrong model state file\n";
    is_valid_ = false;
    return;
  }

  std::string signature(RV32I_MODEL_STATE_SIGNATURE.size(), ' ');
  model_state_file.read(signature.data(), RV32I_MODEL_STATE_SIGNATURE.size() + 1);
  if (signature != RV32I_MODEL_STATE_SIGNATURE) {
    std::cerr << "ERROR: model state file signature mismatch:\n"
              << "      <" << signature << "> vs <"
                                            << RV32I_MODEL_STATE_SIGNATURE <<">\n";
    is_valid_ = false;
    return;
  }

  // read pc
  model_state_file.read(reinterpret_cast<char *>(&pc_), sizeof(uint32_t));
  assert(pc_ % IALIGN == 0 && "PC at unaligned position");

  // the order of initialization is important (see bstate format)
  regs_ = RegisterFile::fromBstate(model_state_file);
  mem_ = MemoryModel::fromBstate(model_state_file);

  is_valid_ = regs_.isValid() && mem_.isValid() && pc_ % IALIGN == 0;
}

void RVModel::init(const MemoryModel& mem_init, const RegisterFile& regs_init, uint32_t pc_init) {
  mem_ = mem_init; regs_ = regs_init; pc_ = pc_init;
  assert(pc_ % IALIGN == 0 && "PC at unaligned position");
  if (pc_ % IALIGN == 0) is_valid_ = true;
}

void RVModel::init(MemoryModel&& mem_init, RegisterFile&& regs_init, uint32_t pc_init) {
  mem_ = mem_init; regs_ = regs_init; pc_ = pc_init;
  assert(pc_ % IALIGN == 0 && "PC at unaligned position");
  if (pc_ % IALIGN == 0) is_valid_ = true;
}

bool RVModel::operator== (const RVModel& other) const {
  return pc_ == other.pc_ && regs_ == other.regs_ && mem_ == other.mem_;
}

uint32_t RVModel::getPC() const {
  return pc_;
}

void RVModel::setPC(uint32_t pc_new) {
  assert(pc_new % IALIGN == 0 && "PC set to unaligned position");
  if (pc_new % IALIGN != 0) is_valid_ = false;

  pc_ = pc_new;
}

bool RVModel::isValid() const { return is_valid_; }

uint8_t  RVModel::readByte(uint32_t addr) const { return mem_.readByte(addr); }
uint16_t RVModel::readHalf(uint32_t addr) const { return mem_.readHalf(addr); }
uint32_t RVModel::readWord(uint32_t addr) const { return mem_.readWord(addr); }

void RVModel::writeByte(uint32_t addr, uint8_t val)  { mem_.writeByte(addr, val); }
void RVModel::writeHalf(uint32_t addr, uint16_t val) { mem_.writeHalf(addr, val); }
void RVModel::writeWord(uint32_t addr, uint32_t val) { mem_.writeWord(addr, val); }

std::unique_ptr<RVISA::IRVInsn> RVModel::decode(uint32_t insn_code) {
  return RVISA::decode(insn_code);
}

void RVModel::execute() {
  std::cerr << "DBG: begin execution (pc = " << pc_ << ")\n";

  execution = true;

  while (execution && is_valid_) {
    uint32_t insn_code = mem_.readWord(pc_); // fetch
    std::unique_ptr<RVISA::IRVInsn> insn = RVISA::decode(insn_code);
    if (!insn) break;

    printInsn(std::cerr, *insn);

    if (insn->getType() == RVISA::RVInsnTypes::UNDEF_TYPE_INSN) {
      break;
    }

    insn->execute(*this);

    // advance if executing, else - do nothing
    setPC(pc_ + sizeof(uint32_t) * execution);
  }

  std::cerr << "DBG: end execution (pc = " << pc_ << ")\n";
}

// todo return control to exec env
void RVModel::exit() {
  execution = false;
}

void RVModel::printInsn(std::ostream& out, const RVISA::IRVInsn& insn) {
  out << insn << ' ' << insn.getName() << " <pc = " << getPC() << ">\n";
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
  fout.write(reinterpret_cast<char *>(&pc_), sizeof(uint32_t));
  regs_.binaryDump(fout);
  mem_.binaryDump(fout);
}

uint32_t RVModel::getReg(Register reg) const {
  return regs_.get(reg);
}

void RVModel::setReg(Register reg, uint32_t val) {
  regs_.set(reg, val);
}

uint32_t RVModel::setUpEnvironment(uint32_t pc_main) {
  assert(pc_main < mem_.size() && "pc of main is set too high");

  uint32_t env_vaddr = mem_.pushSegment(
    ENV_SEG_SIZE,
    RIGHTS_R | RIGHTS_X | RIGHTS_W, // todo discard W right, user vs supervisor mode
    DEFAULT_ALIGN
  );

  mem_.set(env_vaddr, ENV_CODE_BYTE, ENV_SEG_SIZE);

  RVISA::JAL JalMain;
  uint32_t Offset = static_cast<uint32_t>(pc_main)
                  - static_cast<int32_t>(env_vaddr)
                  - sizeof(uint32_t);
  uint32_t Rd = static_cast<uint32_t>(Register::X1);

  // todo check if move ctor is called
  std::vector<uint32_t> JalArgs = createJalArgs(Offset, Rd);
  try {
    JalMain.encode(JalArgs);
  } catch (...) {
    // todo never happening behaviour, but still need smth
  }

  RVISA::EBREAK Ebreak;

  // emit environment code
  writeWord(env_vaddr, JalMain.getOpcode());
  writeWord(env_vaddr + sizeof(uint32_t), Ebreak.getOpcode());

  return env_vaddr;
}

// A GREAT PIECE OF LEGACY HERE WHICH IS GOING TO BE DELETED SOON
// It is left here only to help writing execute() funcs in tablegen
//
// void rvADD::execute(IRVModel& model) const {
  // uint32_t op1 = model.getReg(rs1_);
  // uint32_t op2 = model.getReg(rs2_);
  // model.setReg(dst_, op1 + op2);
// }

// void rvSUB::execute(IRVModel& model) const {
  // uint32_t op1 = model.getReg(rs1_);
  // uint32_t op2 = model.getReg(rs2_);
  // model.setReg(dst_, op1 - op2);
// }

// void rvSLL::execute(IRVModel& model) const {
  // uint32_t op1 = model.getReg(rs1_);
  // uint32_t op2 = model.getReg(rs2_);
  // model.setReg(dst_, op1 << (op2 & MASK_4_0));
// }

// void rvSLT::execute(IRVModel& model) const {
  // uint32_t op1 = model.getReg(rs1_);
  // uint32_t op2 = model.getReg(rs2_);
  // model.setReg(dst_, std::bit_cast<suint32_t>(op1) < std::bit_cast<suint32_t>(op2));
// }

// void rvSLTU::execute(IRVModel& model) const {
  // uint32_t op1 = model.getReg(rs1_);
  // uint32_t op2 = model.getReg(rs2_);
  // model.setReg(dst_, op1 < op2);
// }

// void rvXOR::execute(IRVModel& model) const {
  // uint32_t op1 = model.getReg(rs1_);
  // uint32_t op2 = model.getReg(rs2_);
  // model.setReg(dst_, op1 ^ op2);
// }

// void rvSRL::execute(IRVModel& model) const {
  // uint32_t op1 = model.getReg(rs1_);
  // uint32_t op2 = model.getReg(rs2_);
  // model.setReg(dst_, op1 >> (op2 & MASK_4_0));
// }

// void rvSRA::execute(IRVModel& model) const {
  // uint32_t op1 = model.getReg(rs1_);
  // uint32_t op2 = model.getReg(rs2_);
  // model.setReg(dst_, std::bit_cast<suint32_t>(op1) >> (op2 & MASK_4_0));
// }

// void rvOR::execute(IRVModel& model) const {
  // uint32_t op1 = model.getReg(rs1_);
  // uint32_t op2 = model.getReg(rs2_);
  // model.setReg(dst_, op1 | op2);
// }

// void rvAND::execute(IRVModel& model) const {
  // uint32_t op1 = model.getReg(rs1_);
  // uint32_t op2 = model.getReg(rs2_);
  // model.setReg(dst_, op1 & op2);
// }

// void rvUNDEF_R::execute(IRVModel& model) const {
  // // do nothing
// }

// void rvJALR::execute(IRVModel& model) const {
  // uint32_t ret_addr = model.getPC(); // actual return address will be set at
                                   // // advance pc stage, where pc += 4
  // model.setReg(rd_, ret_addr);

  // uint32_t jmp_addr = model.getReg(rs1_) + sign_extend_12_to_32(imm_);
  // jmp_addr &= 0xFFFF'FFFE; // clear least significant bit
  // model.setPC(jmp_addr);
// }

// void rvLB::execute(IRVModel& model) const {
  // uint32_t mem_addr = model.getReg(rs1_) + sign_extend_12_to_32(imm_);
  // uint8_t mem_val = model.readByte(mem_addr);
  // model.setReg(rd_, sign_extend_8_to_32(mem_val));
// }

// void rvLH::execute(IRVModel& model) const {
  // uint32_t mem_addr = model.getReg(rs1_) + sign_extend_12_to_32(imm_);
  // uint16_t mem_val = model.readHalf(mem_addr);
  // model.setReg(rd_, sign_extend_16_to_32(mem_val));
// }

// void rvLW::execute(IRVModel& model) const {
  // uint32_t mem_addr = model.getReg(rs1_) + sign_extend_12_to_32(imm_);
  // uint32_t mem_val = model.readWord(mem_addr);

  // model.setReg(rd_, mem_val);
// }

// void rvLBU::execute(IRVModel& model) const {
  // uint32_t mem_addr = model.getReg(rs1_) + sign_extend_12_to_32(imm_);
  // uint8_t mem_val = model.readByte(mem_addr);
  // model.setReg(rd_, static_cast<uint32_t>(mem_val));
// }

// void rvLHU::execute(IRVModel& model) const {
  // uint32_t mem_addr = model.getReg(rs1_) + sign_extend_12_to_32(imm_);
  // uint16_t mem_val = model.readHalf(mem_addr);
  // model.setReg(rd_, static_cast<uint32_t>(mem_val));
// }

// void rvADDI::execute(IRVModel& model) const {
  // uint32_t op1 = model.getReg(rs1_);
  // model.setReg(rd_, op1 + sign_extend_12_to_32(imm_));
// }

// void rvSLTI::execute(IRVModel& model) const {
  // uint32_t op1 = model.getReg(rs1_);
  // model.setReg(rd_, std::bit_cast<suint32_t>(op1) < sign_extend_12_to_32(imm_));
// }

// void rvSLTIU::execute(IRVModel& model) const {
  // uint32_t op1 = model.getReg(rs1_);
  // model.setReg(rd_, op1 < std::bit_cast<uint32_t>(sign_extend_12_to_32(imm_)));
// }

// void rvXORI::execute(IRVModel& model) const {
  // suint32_t signed_imm = sign_extend_12_to_32(imm_);
  // uint32_t op1 = model.getReg(rs1_);

  // // Note, “XORI rd, rs1, -1” performs a bitwise logical
  // // inversion of register rs1
  // // (assembler pseudo-instruction NOT rd, rs)
  // //
  // // source: https://msyksphinz-self.github.io/riscv-isadoc/html/rvi.html#lb
  // if (signed_imm == -1)
    // model.setReg(rd_, ~op1);
  // else
    // model.setReg(rd_, op1 ^ signed_imm);
// }

// void rvORI::execute(IRVModel& model) const {
  // uint32_t op1 = model.getReg(rs1_);
  // model.setReg(rd_, op1 | sign_extend_12_to_32(imm_));
// }

// void rvANDI::execute(IRVModel& model) const {
  // uint32_t op1 = model.getReg(rs1_);
  // model.setReg(rd_, op1 & sign_extend_12_to_32(imm_));
// }

// void rvSLLI::execute(IRVModel& model) const {
  // uint32_t op1 = model.getReg(rs1_);
  // uint32_t shamt = imm_ & MASK_4_0;

  // model.setReg(rd_, op1 << shamt);
// }

// void rvSRLI::execute(IRVModel& model) const {
  // uint32_t op1 = model.getReg(rs1_);
  // uint32_t shamt = imm_ & MASK_4_0;

  // model.setReg(rd_, op1 >> shamt);
// }

// void rvSRAI::execute(IRVModel& model) const {
  // uint32_t op1 = model.getReg(rs1_);
  // uint32_t shamt = imm_ & MASK_4_0;

  // model.setReg(rd_, std::bit_cast<suint32_t>(op1) >> shamt);
// }

// void rvUNDEF_I::execute(IRVModel& model) const {
  // // do nothing
// }

// void rvSB::execute(IRVModel& model) const {
  // uint32_t op1 = model.getReg(rs1_);
  // uint32_t op2 = model.getReg(rs2_);
  // uint32_t mem_addr = op1 + sign_extend_12_to_32(imm_);
  // uint8_t val = op2 & 0xFF; // 8 bits mask

  // model.writeByte(mem_addr, val);
// };

// void rvSH::execute(IRVModel& model) const {
  // uint32_t op1 = model.getReg(rs1_);
  // uint32_t op2 = model.getReg(rs2_);
  // uint32_t mem_addr = op1 + sign_extend_12_to_32(imm_);
  // uint16_t val = op2 & 0xFFFF; // 16 bits mask

  // model.writeHalf(mem_addr, val);
// };

// void rvSW::execute(IRVModel& model) const {
  // uint32_t op1 = model.getReg(rs1_);
  // uint32_t op2 = model.getReg(rs2_);
  // uint32_t mem_addr = op1 + sign_extend_12_to_32(imm_);
  // uint32_t val = op2 & 0xFFFF'FFFF; // 32 bits mask

  // model.writeWord(mem_addr, val);
// };

// void rvUNDEF_S::execute(IRVModel& model) const {
  // // do nothing
  // std::cerr << *this << " ??? <pc = " << model.getPC() << ">\n";
// }

// void rvBEQ::execute(IRVModel& model) const {
  // uint32_t op1 = model.getReg(rs1_);
  // uint32_t op2 = model.getReg(rs2_);

  // if (op1 == op2) {
    // uint32_t branch_addr = model.getPC() + sign_extend_13_to_32(imm_);
    // model.setPC(branch_addr);
  // }
// }

// void rvBNE::execute(IRVModel& model) const {
  // uint32_t op1 = model.getReg(rs1_);
  // uint32_t op2 = model.getReg(rs2_);

  // if (op1 != op2) {
    // uint32_t branch_addr = model.getPC() + sign_extend_13_to_32(imm_);
    // model.setPC(branch_addr);
  // }
// }

// void rvBLT::execute(IRVModel& model) const {
  // suint32_t op1 = std::bit_cast<suint32_t>(model.getReg(rs1_));
  // suint32_t op2 = std::bit_cast<suint32_t>(model.getReg(rs2_));

  // if (op1 < op2) {
    // uint32_t branch_addr = model.getPC() + sign_extend_13_to_32(imm_);
    // model.setPC(branch_addr);
  // }
// }

// void rvBLTU::execute(IRVModel& model) const {
  // uint32_t op1 = model.getReg(rs1_);
  // uint32_t op2 = model.getReg(rs2_);

  // if (op1 < op2) {
    // uint32_t branch_addr = model.getPC() + sign_extend_13_to_32(imm_);
    // model.setPC(branch_addr);
  // }
// }

// void rvBGE::execute(IRVModel& model) const {
  // suint32_t op1 = std::bit_cast<suint32_t>(model.getReg(rs1_));
  // suint32_t op2 = std::bit_cast<suint32_t>(model.getReg(rs2_));

  // if (op1 >= op2) {
    // uint32_t branch_addr = model.getPC() + sign_extend_13_to_32(imm_);
    // model.setPC(branch_addr);
  // }
// }

// void rvBGEU::execute(IRVModel& model) const {
  // uint32_t op1 = model.getReg(rs1_);
  // uint32_t op2 = model.getReg(rs2_);

  // if (op1 >= op2) {
    // uint32_t branch_addr = model.getPC() + sign_extend_13_to_32(imm_);
    // model.setPC(branch_addr);
  // }
// }

// void rvUNDEF_B::execute(IRVModel& model) const {
  // // do nothing
// }

// void rvLUI::execute(IRVModel& model) const {
  // model.setReg(rd_, imm_);
// }

// void rvAUIPC::execute(IRVModel& model) const {
  // uint32_t curr_pc = model.getPC();
  // model.setReg(rd_, curr_pc + imm_);
// }

// void rvUNDEF_U::execute(IRVModel& model) const {
  // // do nothing
// }

// void rvJAL::execute(IRVModel& model) const {
  // uint32_t curr_pc = model.getPC();

  // model.setReg(rd_, curr_pc); // actual return address will be set at
                              // // advance pc stage, where pc += 4
  // model.setPC(curr_pc + sign_extend_21_to_32(imm_));
// }

// void rvEBREAK::execute(IRVModel& model) const {
  // model.exit();
// }

// // todo implement handlers
// void rvECALL::execute(IRVModel& model) const {
  // // Arch/ABI	arg1	arg2	arg3	arg4	arg5	arg6	 syscall No
  // // riscv	    a0	  a1	  a2	  a3	  a4	  a5	      a7

  // EESyscall syscall = static_cast<EESyscall>(model.getReg(Register::X17));

  // switch (syscall)
  // {
  // case EESyscall::READ:
  // {
    // std::cerr << "READ SYSCALL" << "\n";
    // std::string input (static_cast<uint8_t>(model.getReg(Register::X11)), '\0');
    // std::cin >> input;
    // std::cerr << input << '\n';
    // break;
  // }
  // case EESyscall::WRITE:
    // std::cerr << "WRITE SYSCALL" << "\n";
    // break;

  // case EESyscall::EXIT:
    // std::cerr << "EXIT SYSCALL" << "\n";
    // model.exit();
    // break;

  // default:
    // std::cerr << "Unknown syscall\n";
    // break;
  // }
// }

// void GeneralUndefInsn::execute(IRVModel& model) const {
  // // do nothing
// }

} // namespace rv32i_sim

#endif // SIMULATOR_HPP

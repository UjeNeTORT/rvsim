#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>
#include <unistd.h>

#include "elfio/elfio.hpp"
#include "spdlog/common.h"
#include "spdlog/spdlog.h"

#include "exec_env.hpp"
#include "instruction.hpp"
#include "io.hpp"
#include "isim.hpp"
#include "memory.hpp"
#include "register_file.hpp"
#include "registers.hpp"

#include "decoder.inc"

namespace elf = ELFIO;

namespace rv32i_sim {
class RVModel final : public IRVModel {
  MemoryModel mem_;
  RegisterFile regs_;
  ExecEnv env_;
  uint32_t pc_;

  uint32_t logs_ = 0;
  bool execution_ = false;
  bool is_valid_ = false;

public:
  RVModel(uint32_t pc = 0) : env_(ExecEnv{}), pc_(pc) {}
  RVModel(std::filesystem::path& ElfPath, uint32_t Logs = 0)
    : RVModel(std::vector<std::string>{ElfPath}, std::make_unique<HostIO>(), Logs) {}
  RVModel(const std::vector<std::string> &ProgArgv, uint32_t Logs = 0)
    : RVModel(ProgArgv, std::make_unique<HostIO>(), Logs) {}

  RVModel(std::filesystem::path& ElfPath,
          std::unique_ptr<IOInterface> IO = std::make_unique<HostIO>(),
          uint32_t Logs = 0)
    : RVModel(std::vector<std::string>{ElfPath}, std::move(IO), Logs) {}

  RVModel(const std::vector<std::string> &ProgArgv,
          std::unique_ptr<IOInterface> IO = std::make_unique<HostIO>(),
          uint32_t Logs = 0) : env_(ExecEnv(std::move(IO))), logs_(Logs) {
    assert(!ProgArgv.empty() && "ProgArgv must not be empty");
    std::filesystem::path ElfPath(ProgArgv[0]);

    setLogs(logs_);

    elf::elfio ElfReader;
    if (!ElfReader.load(ElfPath)) {
      SPDLOG_ERROR("ERROR: failed to load ELF \"{}\"", ElfPath.c_str());
      is_valid_ = false;
      return;
    }

    uint32_t EntryPoint = ElfReader.get_entry();
    SPDLOG_INFO("Found user entry point at: {:#x}", EntryPoint);

    regs_ = RegisterFile();
    mem_ = MemoryModel::fromELF(ElfReader);
    auto LastSegment = std::prev(ElfReader.segments.end());
    uint32_t EnvAddr = LastSegment->get()->get_virtual_address()
                     + LastSegment->get()->get_memory_size();

    // setting up stack and initial stack frame
    uint32_t sp = mem_.setUpStack(ProgArgv);
    regs_.set(Register::SP, sp); // SP = sp
    regs_.set(Register::FP, sp); // FP = sp

    // preparing execution environment i.e.
    // code which calls main and does ebreak in the end
    setUpEnvironment(EntryPoint, EnvAddr);
    pc_ = EnvAddr;

    is_valid_ = mem_.isValid() && regs_.isValid();
  }

  bool operator== (const RVModel& other) const;

  uint32_t getPC() const override;
  void setPC(uint32_t pc_new) override;

private:
  std::unique_ptr<RVISA::IRVInsn> decode(uint32_t insn_code);
  void printInsn(std::ostream& out, const RVISA::IRVInsn& insn);

public:
  bool isValid() const override;

  uint8_t  readByte(uint32_t addr) override;
  uint16_t readHalf(uint32_t addr) override;
  uint32_t readWord(uint32_t addr) override;

  void writeByte(uint32_t addr, uint8_t val) override;
  void writeHalf(uint32_t addr, uint16_t val) override;
  void writeWord(uint32_t addr, uint32_t val) override;
  void memCopy(uint32_t Addr, const void *Src, uint32_t N) override {
    mem_.memCopy(Addr, Src, N);
  }
  void memCopy(void * Dst, uint32_t Addr, uint32_t N) override {
    mem_.memCopy(Dst, Addr, N);
  }

  uint32_t getReg(Register reg) const override { return regs_.get(reg); }
  void     setReg(Register reg, uint32_t val) override { regs_.set(reg, val); }

  float getReg(FPRegister reg) const override { return regs_.getFp(reg); }
  void  setReg(FPRegister reg, float val) override { regs_.setFp(reg, val); }

  IOInterface &io() override { return env_.io(); }

  uint32_t setUpEnvironment(uint32_t MainPC, uint32_t EnvAddr);

  void execute() override;
  void envCall() override;
  void exit() override;

  void setLogs(int logs);

  std::ostream& print(std::ostream& out) override;
};

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

uint8_t  RVModel::readByte(uint32_t addr) { return mem_.readByte(addr); }
uint16_t RVModel::readHalf(uint32_t addr) { return mem_.readHalf(addr); }
uint32_t RVModel::readWord(uint32_t addr) { return mem_.readWord(addr); }

void RVModel::writeByte(uint32_t addr, uint8_t val)  { mem_.writeByte(addr, val); }
void RVModel::writeHalf(uint32_t addr, uint16_t val) { mem_.writeHalf(addr, val); }
void RVModel::writeWord(uint32_t addr, uint32_t val) { mem_.writeWord(addr, val); }

std::unique_ptr<RVISA::IRVInsn> RVModel::decode(uint32_t insn_code) {
  return RVISA::decode(insn_code);
}

void RVModel::execute() {
  SPDLOG_INFO("begin execution <pc = {:#x}>", pc_);

  execution_ = true;

  while (execution_ && is_valid_) {
    uint32_t insn_code = mem_.readWord(pc_); // fetch
    std::unique_ptr<RVISA::IRVInsn> insn = RVISA::decode(insn_code);
    if (!insn) break;

    if (logs_ == 2) printInsn(std::cerr, *insn);

    if (insn->getType() == RVISA::RVInsnTypes::UNDEF_TYPE_INSN) {
      break;
    }

    insn->execute(*this);

    if (!execution_) break;
  }

  SPDLOG_INFO("end execution <pc = {:#x}>", pc_);
}

void RVModel::envCall() {
  env_.ecall(static_cast<EESyscall>(getReg(Register::A7)), *this);
}

// todo return control to exec env
// todo rename
// todo make possible step by step debugging
void RVModel::exit() {
  execution_ = false;
}

void RVModel::setLogs(int logs) {
  logs_ = logs;
  if (logs_ == 0) spdlog::set_level(spdlog::level::err);
  else if (logs_ == 1 || logs_ == 2) {
    spdlog::set_level(spdlog::level::err);
    spdlog::set_level(spdlog::level::critical);
    spdlog::set_level(spdlog::level::info);
  }

}

void RVModel::printInsn(std::ostream& out, const RVISA::IRVInsn& insn) {
  out << insn << ' ' << insn.getName() << " <pc = "
      << std::hex << getPC() << std::dec << ">\n";
}

std::ostream& RVModel::print(std::ostream& out) {
  out << "pc = " << getPC() << '\n';
  regs_.print(out);
  mem_.print(out);
  return out;
}

uint32_t RVModel::setUpEnvironment(uint32_t MainPC, uint32_t EnvAddr) {
  assert(MainPC < mem_.size() && "main's address is set too high");

  mem_.pushSegment(
    Segment(EnvAddr, ENV_SEG_SIZE, RIGHTS_R | RIGHTS_X | RIGHTS_W, DEFAULT_ALIGN)
  );

  mem_.memSet(EnvAddr, ENV_CODE_BYTE, ENV_SEG_SIZE);

  RVISA::JAL JalMain;
  uint32_t Offset = MainPC - EnvAddr;
  uint32_t Rd = static_cast<uint32_t>(Register::X1);

  std::vector<uint32_t> JalArgs = createJalArgs(Offset, Rd);
  try {
    JalMain.encode(JalArgs);
  } catch (...) {
    // todo never happening behaviour, but still need smth
  }

  RVISA::EBREAK Ebreak;

  // emit environment code
  writeWord(EnvAddr, JalMain.getOpcode());
  writeWord(EnvAddr + sizeof(uint32_t), Ebreak.getOpcode());

  return EnvAddr + ENV_SEG_SIZE;
}

} // namespace rv32i_sim

#endif // SIMULATOR_HPP

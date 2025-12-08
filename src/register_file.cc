#include "register_file.hpp"

#include <bit>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

#include "spdlog/spdlog.h"

namespace rv32i_sim {

RegisterFile::RegisterFile(bool valid) : is_valid_(valid) {}
RegisterFile::RegisterFile(std::vector<uint32_t> intRegs, std::vector<float> fpRegs, bool valid) :
                                              intRegs_(intRegs), fpRegs_(fpRegs), is_valid_(valid) {
  if (intRegs.size() != N_REGS) {
    std::cerr << "WARNING: initial regs state has " << intRegs.size()
              << " vs " << N_REGS <<" needed, others will be set to zero\n";

    intRegs_.resize(N_REGS);
  }

  if (fpRegs.size() != N_FPREGS) {
    std::cerr << "WARNING: initial fp regs state has " << fpRegs.size()
              << " vs " << N_FPREGS <<" needed, others will be set to zero\n";

    fpRegs_.resize(N_FPREGS);
  }
}

bool RegisterFile::isValid() const { return is_valid_; }

// check validity and return is_valid
bool RegisterFile::validate() {
  if (!is_valid_) return false;

  is_valid_ = intRegs_[0] == 0;
  return is_valid_;
}


bool RegisterFile::operator==(const RegisterFile& other) const {
  return intRegs_ == other.intRegs_
      && fpRegs_ == other.fpRegs_;
}

void RegisterFile::set(Register reg, int32_t val) {
  if (reg == Register::X0) return;

  SPDLOG_INFO("set x[{}] <- {} ({:#x})",
    static_cast<uint8_t>(reg), val, std::bit_cast<uint32_t>(val));
  intRegs_[static_cast<uint8_t>(reg)] = val;
}

void RegisterFile::setFp(FPRegister reg, float val) {
  SPDLOG_INFO("set f[{}] <- {} ({:#x})",
    static_cast<uint8_t>(reg), val, std::bit_cast<uint32_t>(val));
  fpRegs_[static_cast<uint8_t>(reg)] = val;
}

uint32_t RegisterFile::get(Register reg) const {
  assert(static_cast<uint32_t>(intRegs_[0]) == 0 && "Register X0 not zero");

  uint32_t Res = intRegs_[static_cast<uint8_t>(reg)];
  SPDLOG_INFO("get x[{}] -> {} ({:#x})",
    static_cast<uint8_t>(reg), Res, Res);

  return Res;
}

float RegisterFile::getFp(FPRegister reg) const {
  float Res = fpRegs_[static_cast<uint8_t>(reg)];
  SPDLOG_INFO("get f[{}] -> {} ({:#x})",
    static_cast<uint8_t>(reg), Res, std::bit_cast<uint32_t>(Res));
  return Res;
}

std::ostream& RegisterFile::print(std::ostream& out) {
  for (int i = 0; i != N_REGS; ++i)
    out << "X" << i << " = " << std::hex << intRegs_[i] << '\n' << std::dec;

  for (int i = 0; i != N_FPREGS; ++i)
    out << "F" << i << " = " << std::hex << fpRegs_[i]
                             << std::dec << "(" << fpRegs_[i] << ")\n";

  return out;
}

std::ostream& operator<<(std::ostream& out, RegisterFile& rf) {
  rf.print(out);
  return out;
}

std::ostream& operator<< (std::ostream& out, Register reg) {
  out << static_cast<uint8_t>(reg);
  return out;
}

std::ostream& operator<< (std::ostream& out, FPRegister reg) {
  out << static_cast<uint8_t>(reg);
  return out;
}

} // namespace rv32i_sim

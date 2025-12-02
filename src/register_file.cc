#include "register_file.hpp"

#include <cassert>
#include <iostream>
#include <vector>

namespace rv32i_sim {

RegisterFile::RegisterFile(bool valid) : is_valid_(valid) {}
RegisterFile::RegisterFile(std::vector<uint32_t> regs, bool valid) :
                                              regs_(regs), is_valid_(valid) {
  if (regs.size() != N_REGS) {
    std::cerr << "WARNING: initial regs state has " << regs.size()
              << " vs " << N_REGS <<" needed, others will be set to zero\n";

    regs_.resize(N_REGS);
  }
}

bool RegisterFile::isValid() const { return is_valid_; }

// check validity and return is_valid
bool RegisterFile::validate() {
  if (!is_valid_) return false;

  is_valid_ = regs_[0] == 0;
  return is_valid_;
}


bool RegisterFile::operator==(const RegisterFile& other) const {
  return regs_ == other.regs_;
}

void RegisterFile::set(Register reg, int32_t val) {
  if (reg == Register::X0) return;

  regs_[static_cast<uint8_t>(reg)] = val;
}

uint32_t RegisterFile::get(Register reg) const {
  assert(static_cast<uint32_t>(regs_[0]) == 0 && "Register X0 not zero");

  return regs_[static_cast<uint8_t>(reg)];
}

std::ostream& RegisterFile::print(std::ostream& out) {
  for (int i = 0; i != N_REGS; ++i)
    out << "X" << i << " = " << std::hex << regs_[i] << '\n' << std::dec;

  return out;
}

std::ostream& operator<<(std::ostream& out, RegisterFile& rf) {
  rf.print(out);
  return out;
}

bool isRegValid(Register reg) {
  return (Register::X0 <= reg) && (reg <= Register::X31);
}

std::ostream& operator<< (std::ostream& out, Register reg) {
  out << static_cast<uint8_t>(reg);
  return out;
}

} // namespace rv32i_sim

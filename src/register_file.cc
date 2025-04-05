#include "register_file.hpp"

#include <cassert>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <vector>

namespace rv32i_sim {

RegisterFile::RegisterFile(bool valid) : is_valid_(valid) {}
RegisterFile::RegisterFile(std::vector<addr_t> regs, bool valid) :
                                              regs_(regs), is_valid_(valid) {
  if (regs.size() != N_REGS) {
    std::cerr << "WARNING: initial regs state has " << regs.size()
              << " vs " << N_REGS <<" needed, others will be set to zero\n";

    regs_.resize(N_REGS);
  }
}

// construct from bstate file format
RegisterFile::RegisterFile(std::ifstream& regs_bstate) {
  if (!regs_bstate) {
    std::cerr << "ERROR: wrong registers file\n";
    is_valid_ = false;
    return;
  }

  std::string signature(RV32I_REGS_STATE_SIGNATURE.size(), ' ');
  regs_bstate.read(signature.data(), RV32I_REGS_STATE_SIGNATURE.size() + 1);

  if (signature != RV32I_REGS_STATE_SIGNATURE) {
    std::cerr << "ERROR: regs state file signature mismatch:\n"
              << "<" << signature << "> vs <" << RV32I_REGS_STATE_SIGNATURE <<">\n";
    is_valid_ = false;
    return;
  }

  is_valid_ = true;

  regs_bstate.read(reinterpret_cast<char *>(regs_.data()), sizeof(word_t) * N_REGS);

  validate();
  assert(is_valid_ && "Registers invalid after creation");
}

RegisterFile RegisterFile::fromBstate(std::ifstream& regs_file) {
  return RegisterFile(regs_file);
}

RegisterFile RegisterFile::fromBstate(std::filesystem::path& regs_file_path) {
  std::ifstream regs_bstate(regs_file_path);
  return RegisterFile(regs_bstate);
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

void RegisterFile::set(Register reg, sword_t val) {
  if (reg == Register::X0) return;

  regs_[static_cast<uint8_t>(reg)] = val;
}

addr_t RegisterFile::get(Register reg) const {
  assert(static_cast<addr_t>(regs_[0]) == 0 && "Register X0 not zero");

  return regs_[static_cast<uint8_t>(reg)];
}

std::ostream& RegisterFile::print(std::ostream& out) {
  for (int i = 0; i != N_REGS; ++i) {
    out << "X" << i << " = "
        << std::hex << regs_[i] << '\n'
        << std::dec;
  }

  return out;
}

void RegisterFile::binaryDump(std::ofstream& fout) {
  if (!fout) {
    std::cerr << "ERROR: wrong output file for registers binary dump\n";
    return;
  }

  fout.write(RV32I_REGS_STATE_SIGNATURE.c_str(),
              RV32I_REGS_STATE_SIGNATURE.size() + 1);

  fout.write(reinterpret_cast<char *>(regs_.data()), regs_.size() * sizeof(word_t));
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

} // rv32i_sim

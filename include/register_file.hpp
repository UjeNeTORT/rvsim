#ifndef REGISTER_FILE_HPP
#define REGISTER_FILE_HPP

#include <cassert>
#include <iostream>

#include "encoding.hpp"
#include "registers.hpp"

namespace rv32i_sim {

const std::string RV32I_REGS_STATE_SIGNATURE = "RV32I_REG_STATE";

class RegisterFile {
  std::vector<addr_t> regs_ = std::vector<addr_t>(N_REGS);
  bool is_valid_ = false;


public:
  RegisterFile(bool valid = false) : is_valid_(valid) {}
  RegisterFile(std::vector<addr_t> regs, bool valid = true) :
                                                regs_(regs), is_valid_(valid) {
    if (regs.size() != N_REGS) {
      std::cerr << "WARNING: initial regs state has " << regs.size()
                << " vs " << N_REGS <<" needed, others will be set to zero\n";

      regs_.resize(N_REGS);
    }
  }

  // construct from bstate file format
  RegisterFile(std::ifstream& regs_bstate) {
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

    regs_bstate.read(reinterpret_cast<char *>(regs_.data()), sizeof(word_t) * N_REGS);

    assert(regs_[0] == 0 && "X0 must be zero");
    if (regs_[0] != 0) is_valid_ = false;
    else is_valid_ = true;
  }

  static RegisterFile fromBstate(std::ifstream& regs_file) {
    return RegisterFile(regs_file);
  }

  static RegisterFile fromBstate(std::filesystem::path& regs_file_path) {
    std::ifstream regs_bstate(regs_file_path);
    return RegisterFile(regs_bstate);
  }

  bool isValid() const { return is_valid_; }

  bool operator==(const RegisterFile& other) const {
    return regs_ == other.regs_;
  }

  void set(Register reg, sword_t val) {
    if (reg != Register::X0)
      regs_[static_cast<uint8_t>(reg)] = val;

    assert(get(Register::X0) == 0);
  }

  addr_t get(Register reg) const {
    assert(static_cast<addr_t>(regs_[0]) == 0 && "Register X0 not zero");

    return regs_[static_cast<uint8_t>(reg)];
  }

  std::ostream& print(std::ostream& out) {
    for (int i = 0; i != N_REGS; ++i) {
      out << "X" << i << " = "
          << std::hex << regs_[i] << '\n'
          << std::dec;
    }

    return out;
  }

  void binaryDump(std::ofstream& fout) {
    if (!fout) {
      std::cerr << "ERROR: wrong output file for registers binary dump\n";
      return;
    }

    fout.write(RV32I_REGS_STATE_SIGNATURE.c_str(),
               RV32I_REGS_STATE_SIGNATURE.size() + 1);

    fout.write(reinterpret_cast<char *>(regs_.data()), regs_.size() * sizeof(word_t));
  }
};

std::ostream& operator<<(std::ostream& out, RegisterFile& rf) {
  rf.print(out);
  return out;
}

} // rv32i_sim

#endif // REGISTER_FILE_HPP

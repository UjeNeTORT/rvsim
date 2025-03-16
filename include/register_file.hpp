#ifndef REGISTER_FILE_HPP
#define REGISTER_FILE_HPP

#include <iostream>

#include "encoding.hpp"
#include "registers.hpp"

namespace rv32i_sim {

const std::string RV32I_REGS_STATE_SIGNATURE = "RV32I_REG_STATE";

class RegisterFile {
  std::array<word_t, N_REGS> regs_ = {};

public:
  RegisterFile() {}

  RegisterFile(std::array<word_t, N_REGS> init_regs_state) : regs_(init_regs_state) {}

  RegisterFile(std::ifstream& regs_file) {
    if (!regs_file) {
      std::cerr << "ERROR: wrong registers file\n";
      return;
    }

    std::string signature(RV32I_REGS_STATE_SIGNATURE.size(), ' ');
    regs_file.read(signature.data(), RV32I_REGS_STATE_SIGNATURE.size() + 1);

    if (signature != RV32I_REGS_STATE_SIGNATURE) {
      std::cerr << "ERROR: regs state file signature mismatch:\n"
                << "      <" << signature << "> vs <" << RV32I_REGS_STATE_SIGNATURE <<">\n";
      return;
    }

    for (int i = 0; i != N_REGS; ++i) {
      regs_file.read(reinterpret_cast<char *>(&regs_[i]), sizeof(word_t));
    }
  }

  void set(Register reg, addr_t val) {
    regs_[static_cast<uint8_t>(reg)] = val;
  }

  addr_t get(Register reg) {
    return regs_[static_cast<uint8_t>(reg)];
  }

  std::ostream& dump(std::ostream& out) {
    for (int i = 0; i != N_REGS; ++i) {
      out << "X" << i << " = "
          << std::hex << regs_[i] << '\n'
          << std::dec;
    }

    return out;
  }

  void binary_dump(std::ofstream& fout) {
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
  rf.dump(out);
  return out;
}

} // rv32i_sim

#endif // REGISTER_FILE_HPP

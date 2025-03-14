#ifndef REGISTER_FILE_HPP
#define REGISTER_FILE_HPP

#include <iostream>

#include "encoding.hpp"
#include "registers.hpp"

namespace rv32i_sim {

class RegisterFile {
  std::array<word_t, N_REGS> regs_ = {};

public:
  RegisterFile() {}

  RegisterFile(std::array<word_t, N_REGS> init_regs_state) : regs_(init_regs_state) {}

  RegisterFile(std::ifstream regs_file) {
    if (!regs_file) {
      std::cerr << "ERROR: wrong registers file\n";
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
};

std::ostream& operator<<(std::ostream& out, RegisterFile& rf) {
  rf.dump(out);
  return out;
}

} // rv32i_sim

#endif // REGISTER_FILE_HPP

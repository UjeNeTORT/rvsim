#ifndef REGISTER_FILE_HPP
#define REGISTER_FILE_HPP

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include "registers.hpp"

namespace rv32i_sim {

const std::string RV32I_REGS_STATE_SIGNATURE = "RV32I_REG_STATE";

class RegisterFile final {
  std::vector<uint32_t> regs_ = std::vector<uint32_t>(N_REGS);
  bool is_valid_ = false;

public:
  RegisterFile(bool valid = true);
  RegisterFile(std::vector<uint32_t> regs, bool valid = true);

  // construct from bstate file format
  RegisterFile(std::ifstream& regs_bstate);

  static RegisterFile fromBstate(std::ifstream& regs_file);
  static RegisterFile fromBstate(std::filesystem::path& regs_file_path);

  // return current validity
  bool isValid() const;

  // verify validity and return is_valid
  bool validate();

  bool operator==(const RegisterFile &other) const;

  void set(Register reg, int32_t val);
  uint32_t get(Register reg) const;

  std::ostream &print(std::ostream &out);
  void binaryDump(std::ofstream &fout);
};

bool isRegValid(Register reg);

std::ostream &operator<<(std::ostream &out, RegisterFile &rf);
std::ostream &operator<<(std::ostream &out, Register reg);

} // rv32i_sim

#endif // REGISTER_FILE_HPP

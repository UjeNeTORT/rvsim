#ifndef REGISTER_FILE_HPP
#define REGISTER_FILE_HPP

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include "encoding.hpp"
#include "registers.hpp"

namespace rv32i_sim {

const std::string RV32I_REGS_STATE_SIGNATURE = "RV32I_REG_STATE";

class RegisterFile final {
  std::vector<addr_t> regs_ = std::vector<addr_t>(N_REGS);
  bool is_valid_ = false;

public:
  RegisterFile(bool valid = true);
  RegisterFile(std::vector<addr_t> regs, bool valid = true);

  // construct from bstate file format
  RegisterFile(std::ifstream& regs_bstate);

  static RegisterFile fromBstate(std::ifstream& regs_file);
  static RegisterFile fromBstate(std::filesystem::path& regs_file_path);

  // return current validity
  bool isValid() const;

  // verify validity and return is_valid
  bool validate();

  bool operator==(const RegisterFile& other) const;

  void set(Register reg, sword_t val);
  addr_t get(Register reg) const;

  std::ostream& print(std::ostream& out);
  void binaryDump(std::ofstream& fout);
};

std::ostream& operator<<(std::ostream& out, RegisterFile& rf);

bool isRegValid(Register reg);

std::ostream& operator<< (std::ostream& out, Register reg);

} // rv32i_sim

#endif // REGISTER_FILE_HPP

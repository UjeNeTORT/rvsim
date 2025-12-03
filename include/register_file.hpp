#ifndef REGISTER_FILE_HPP
#define REGISTER_FILE_HPP

#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

#include "registers.hpp"

namespace rv32i_sim {

class RegisterFile final {
  std::vector<uint32_t> intRegs_ = std::vector<uint32_t>(N_REGS);
  std::vector<float> fpRegs_ = std::vector<float>(N_FPREGS);
  bool is_valid_ = false;

public:
  RegisterFile(bool valid = true);
  RegisterFile(std::vector<uint32_t> intRegs, std::vector<float> fpRegs, bool valid = true);

  // return current validity
  bool isValid() const;

  // verify validity and return is_valid
  bool validate();

  bool operator==(const RegisterFile &other) const;

  void set(Register reg, int32_t val);
  void setFp(FPRegister reg, float val);

  uint32_t get(Register reg) const;
  float getFp(FPRegister reg) const;

  std::ostream &print(std::ostream &out);
};

std::ostream &operator<<(std::ostream &out, RegisterFile &rf);
std::ostream &operator<<(std::ostream &out, Register reg);
std::ostream &operator<<(std::ostream &out, FPRegister reg);

} // namespace rv32i_sim

#endif // REGISTER_FILE_HPP

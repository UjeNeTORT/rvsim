#include <iostream>

#include "sim.hpp"

int main() {
  rv32i_sim::RVInsn insn = rv32i_sim::RVInsn{0x00c5f533};
  std::cout << insn << ' ' << std::endl;
  rv32i_sim::RVInsn insn2 = rv32i_sim::RVInsn{insn.code_opless()};
  std::cout << insn2 << ' ' << std::endl;
  return 0;
}

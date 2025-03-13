#include <iostream>

#include "sim.hpp"

int main() {
  rv32i_sim::MemoryModel memory;
  memory.writeWord(0, 0x00c58533);
  memory.writeWord(sizeof(rv32i_sim::word_t), 0x00c5f6b3);
  memory.writeWord(2 * sizeof(rv32i_sim::word_t), 0x00c5f6b3);

  rv32i_sim::RegisterFile regs;
  regs.set(rv32i_sim::Register::X11, 0x1d);
  regs.set(rv32i_sim::Register::X12, 0x03);

  rv32i_sim::RVModel model{memory, regs, 0};

  model.setPC(0);
  model.execute();

  std::cout << model << '\n';

  return 0;
}

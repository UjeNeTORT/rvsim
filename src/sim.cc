#include <iostream>

#include "sim.hpp"

int main() {
  // initialize with separate memory and regs
  std::ifstream memf{std::filesystem::path{"mem.bin"}};
  std::ifstream regf{std::filesystem::path{"regs.bin"}};
  rv32i_sim::MemoryModel mem{memf};
  rv32i_sim::RegisterFile regs{regf};

  rv32i_sim::RVModel model{mem, regs, 0x34};
  std::ofstream fout{"aboba.bin", std::ios::out | std::ios::binary};

  // full model binary dump to a file
  model.binary_dump(fout);
  fout.close();

  // read binary dump from the file
  std::ifstream fin{"aboba.bin", std::ios::in | std::ios::binary};
  rv32i_sim::RVModel mnew{fin};
  fin.close();
  mnew.execute(); // execute
  // ... and dump to another file
  std::ofstream fout1{"aboba1.bin", std::ios::out | std::ios::binary};
  mnew.binary_dump(fout1);
  fout1.close();

  std::ofstream fout0{"aboba0.bin", std::ios::out | std::ios::binary};
  model.execute();
  model.binary_dump(fout0);
  fout0.close();

  // dump human-readable to stdout
  std::cout << mnew << '\n';

  // after that can do
  // cmp aboba0.bin aboba1.bin
  return 0;
}

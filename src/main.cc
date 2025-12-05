#include <cstdint>
#include <iostream>
#include <filesystem>

#include <cxxopts.hpp>

#include "sim.hpp"

int main(int argc, char *argv[]) {
  uint32_t logs = 0;
  uint32_t pc_init = 0;
  std::filesystem::path elf_path;

  cxxopts::Options options("rv32isim", "RISC-V RV32I functional simulator");

  options.add_options()
      ("h,help", "print help message")
      ("pc",     "initial pc",
                  cxxopts::value<uint32_t>(pc_init))
      ("elf",    "run simulator on an ELF file",
                  cxxopts::value<std::filesystem::path>(elf_path))
      ("logs",   "set logs verbosity level (0-2)",
                  cxxopts::value<uint32_t>(logs)->default_value("0"))
  ;

  auto result = options.parse(argc, argv);

  if (result.count("help")) {
    std::cout << options.help() << '\n';
    return 0;
  }
  rv32i_sim::RVModel model;

  if (result.count("elf")) {
    model = rv32i_sim::RVModel(elf_path, logs);
  }
  if (result.count("pc")) {
    std::cerr << "Warning: overriding entry point pc = "
              << model.getPC() << "with " << pc_init << '\n';
    model.setPC(pc_init);
  }
  if (!model.isValid()) {
    std::cerr << "Error: model invalid, cannot execute\n";
    return 1;
  }

  model.execute();

  return 0;
}

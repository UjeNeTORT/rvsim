#include <cstdint>
#include <iostream>
#include <filesystem>

#include <cxxopts.hpp>
#include <vector>

#include "sim.hpp"

int main(int argc, char *argv[]) {
  uint32_t logs = 0;
  uint32_t pc_init = 0;

  cxxopts::Options options("rv32isim", "RISC-V RV32I functional simulator");

  options.add_options()
      ("h,help", "print help message")
      ("pc",     "initial pc",
                  cxxopts::value<uint32_t>(pc_init))
      ("argv",    "argv of elf to run",
                  cxxopts::value<std::vector<std::string>>())
      ("logs",   "set logs verbosity level (0-2)",
                  cxxopts::value<uint32_t>(logs)->default_value("0"))
  ;

  options.parse_positional({"argv"});
  auto result = options.parse(argc, argv);

  if (result.count("help")) {
    std::cout << options.help() << '\n';
    return 0;
  }


  std::vector<std::string> ProgArgv = result["argv"].as<std::vector<std::string>>();
  for (auto &s : ProgArgv) std::cout << s << "\n";

  if (ProgArgv.empty()) {
    std::cout << "No input elf provided, cannot execute\n";
    std::cout << options.help() << '\n';
    return 0;
  }

  rv32i_sim::RVModel Model(ProgArgv, logs);

  if (result.count("pc")) {
    std::cerr << "Warning: overriding entry point pc = "
              << Model.getPC() << "with " << pc_init << '\n';
    Model.setPC(pc_init);
  }

  if (!Model.isValid()) {
    std::cerr << "Error: model invalid, cannot execute\n";
    return 1;
  }

  Model.execute();

  return 0;
}

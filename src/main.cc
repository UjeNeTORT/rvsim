#include <cstdint>
#include <iostream>
#include <filesystem>

#include <boost/program_options.hpp>

#include "sim.hpp"

namespace po = boost::program_options;

int main(int argc, char *argv[]) {
  uint32_t logs = 0;
  uint32_t pc_init = 0;
  std::filesystem::path elf_path;

  po::options_description optns_desc{"Possible options"};
  optns_desc.add_options()
    ("help", "print help message")

    ("pc", po::value<uint32_t>(&pc_init), "initial pc")

    ("elf", po::value<std::filesystem::path>(&elf_path),
        "run simulator on an ELF file")

    ("logs", po::value<uint32_t>(&logs)->default_value(0),
             "set logs verbosity level (0 - disabled,\n"
             "                          1 - enabled,\n"
             "                          2 - debug)\n")
  ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, optns_desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << optns_desc << '\n';
    return 0;
  }
  rv32i_sim::RVModel model;

  if (vm.count("elf")) {
    model = rv32i_sim::RVModel(elf_path, logs);
  }
  if (vm.count("pc")) {
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

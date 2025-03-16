#include <iostream>
#include <filesystem>

#include <boost/program_options.hpp>

#include "sim.hpp"

namespace po = boost::program_options;

int main(int argc, char *argv[]) {

  bool checkpoints = false;
  int logs = 0;
  std::filesystem::path istate;
  std::filesystem::path ostate;
  std::filesystem::path imem;
  std::filesystem::path omem;
  std::filesystem::path iregs;
  std::filesystem::path oregs;

  po::options_description optns_desc{"Possible options"};
  optns_desc.add_options()
    ("help", "print help message")

    ("istate", po::value<std::filesystem::path>(&istate),
        "input simulator state from a binary file of specific "
        "format at the beginning of execution")

    ("ostate", po::value<std::filesystem::path>(&ostate),
        "output simulator state to a binary file of specific "
        "format at the end of execution")

    ("iregs", po::value<std::filesystem::path>(&iregs),
        "input simulator registers from a binary file at the beginning of execution")

    ("oregs", po::value<std::filesystem::path>(&oregs),
        "output simulator registers to a binary file at the end of execution")

    ("imem", po::value<std::filesystem::path>(&imem),
        "input simulator memory from a binary file at the beginning of execution")

    ("omem", po::value<std::filesystem::path>(&omem),
        "output simulator memory to a binary file at the end of execution")

    ("elf", po::value<std::filesystem::path>(),
        "run simulator on an elf file")

    ("logs", po::value<int>(&logs)->default_value(0),
             "set logs verbosity level (0 - logs disabled, \n"
             "                          1 - enabled, \n"
             "                          2 - super-hyper-verbose mode)")

    ("checkpoints", po::value<bool>(&checkpoints)->default_value(false),
                    "record checkpoints (after each insn execution "
                    "do a mega dump of full sim state)")
  ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, optns_desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << optns_desc << '\n';
    return 0;
  }

  rv32i_sim::RVModel model{};

  if (vm.count("istate")) {
    std::ifstream model_state_file{istate};
    if (!model_state_file) {
      std::cerr << "ERROR: wrong istate file\n";
      return 1;
    }

    model.init(model_state_file);
  }

  model.execute();

  if (vm.count("ostate")) {
    std::ofstream model_state_file{ostate};
    if (!model_state_file) {
      std::cerr << "ERROR: wrong ostate file\n";
      return 1;
    }

    model.binary_dump(model_state_file);
  }

  return 0;
}

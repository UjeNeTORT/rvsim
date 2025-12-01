#include <iostream>
#include <filesystem>

#include <gtest/gtest.h>

#include "sim.hpp"
#include "io.hpp"

class TestRVModel : public ::testing::Test {

protected:
  std::filesystem::path testf_path_;
  rv32i_sim::RVModel model;
  rv32i_sim::RVModel ref_model;

  virtual void SetUp() {
    model = rv32i_sim::RVModel{};
  }

  virtual void TearDown() {}

  bool RunTest(std::filesystem::path testf_path) {
    model = rv32i_sim::RVModel(testf_path, 0);
    if (!model.isValid()) {
      std::cerr << "ERROR: failed to initialize model correctly\n";
      std::cerr << testf_path << '\n';
      return false;
    }

    model.execute();
    if (!model.isValid()) {
      std::cerr << "ERROR: model invalid after execution \n";
      std::cerr << testf_path << '\n';
      return false;
    }

    return true;
  }

  bool TestAnsELF(std::filesystem::path elf_path) {
    std::filesystem::path ansf_path = elf_path;
    ansf_path.replace_extension(".ans");

    model = rv32i_sim::RVModel(elf_path, 0);

    if (!model.isValid()) {
      std::cerr << "ERROR: failed to initialize model correctly\n";
      std::cerr << elf_path << '\n';
      return false;
    }

    model.execute();
    if (!model.isValid()) {
      std::cerr << "ERROR: model invalid after execution\n";
      std::cerr << elf_path << '\n';
      return false;
    }

    return ref_model == model;
  }
};

// #define TEST_F_INSTRUCTION(InstructionName, TestDirPath)                \
  // TEST_F(TestRVModel, InstructionName) {                                \
    // std::filesystem::path TestDir = TestDirPath;                        \
    // for (auto const &DirEntry :                                         \
                        // std::filesystem::directory_iterator(TestDir)) { \
      // if (!DirEntry.is_regular_file()) continue;                        \
      // if (DirEntry.path().extension() != ".bstate") continue;           \
      // auto fpath = DirEntry.path();                                     \
      // EXPECT_EQ(TestAnsBstate(fpath), true);                            \
    // }                                                                   \
  // }

// TEST_F_INSTRUCTION(ADD, "../test/insn/add");
// TEST_F_INSTRUCTION(SUB, "../test/insn/sub");
// TEST_F_INSTRUCTION(SLL, "../test/insn/sll");
// TEST_F_INSTRUCTION(SLT, "../test/insn/slt");
// TEST_F_INSTRUCTION(SLTU, "../test/insn/sltu");
// TEST_F_INSTRUCTION(XOR, "../test/insn/xor");
// TEST_F_INSTRUCTION(SRA, "../test/insn/sra");
// TEST_F_INSTRUCTION(OR, "../test/insn/or");
// TEST_F_INSTRUCTION(AND, "../test/insn/and");

// #undef TEST_F_INSTRUCTION


TEST_F(TestRVModel, ELF_PLUS) {
  std::filesystem::path test_dir = "../test/elf/plus";
  for (auto const &dir_entry :
                      std::filesystem::directory_iterator(test_dir)) {
    if (!dir_entry.is_regular_file()) continue;
    if (dir_entry.path().extension() != ".elf") continue;
    auto fpath = dir_entry.path();

    EXPECT_EQ(TestAnsELF(fpath), true);
  }
}

TEST_F(TestRVModel, FACTORIAL) {
  std::filesystem::path fpath = "../test/elf/factorial.elf";
  EXPECT_EQ(TestAnsELF(fpath), true);
}

TEST_F(TestRVModel, DISABLED_stress) {
  std::filesystem::path test_dir = "../test/stress";
  for (auto const &dir_entry :
                      std::filesystem::recursive_directory_iterator(test_dir)) {
    if (!dir_entry.is_regular_file()) continue;
    if (dir_entry.path().extension() != ".bstate") continue;
    auto fpath = dir_entry.path();

    EXPECT_EQ(RunTest(fpath), true);
  }
}

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

#include <iostream>
#include <filesystem>

#include <gtest/gtest.h>

#include "sim.hpp"
#include "io.hpp"

class TestRVModel : public ::testing::Test {

protected:
  std::filesystem::path TestPath_;
  rv32i_sim::RVModel M_;

  virtual void SetUp() {
    M_ = rv32i_sim::RVModel{};
  }

  virtual void TearDown() {}

  bool RunTest(std::filesystem::path TestPath) {
    M_ = rv32i_sim::RVModel(TestPath, 0);
    if (!M_.isValid()) {
      std::cerr << "ERROR: failed to initialize model correctly\n";
      std::cerr << TestPath << '\n';
      return false;
    }

    M_.execute();
    if (!M_.isValid()) {
      std::cerr << "ERROR: model invalid after execution \n";
      std::cerr << TestPath << '\n';
      return false;
    }

    return true;
  }

  bool TestAnsELF(std::filesystem::path ElfPath) {
    std::filesystem::path AnsPath = ElfPath;
    AnsPath.replace_extension(".ans");

    M_ = rv32i_sim::RVModel(ElfPath, 0);

    if (!M_.isValid()) {
      std::cerr << "ERROR: failed to initialize model correctly\n";
      std::cerr << ElfPath << '\n';
      return false;
    }

    M_.execute();
    if (!M_.isValid()) {
      std::cerr << "ERROR: model invalid after execution\n";
      std::cerr << ElfPath << '\n';
      return false;
    }

    return M_.isValid(); // todo fixme, should be another criteria
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
  std::filesystem::path TestDir = "../test/elf/plus";
  for (auto const &DirEnt :
                      std::filesystem::directory_iterator(TestDir)) {
    if (!DirEnt.is_regular_file()) continue;
    if (DirEnt.path().extension() != ".elf") continue;
    auto FPath = DirEnt.path();

    EXPECT_EQ(TestAnsELF(FPath), true);
  }
}

TEST_F(TestRVModel, FACTORIAL) {
  std::filesystem::path FPath = "../test/elf/factorial.elf";
  EXPECT_EQ(TestAnsELF(FPath), true);
}

TEST_F(TestRVModel, DISABLED_stress) {
  std::filesystem::path TestDir = "../test/stress";
  for (auto const &DirEnt :
                      std::filesystem::recursive_directory_iterator(TestDir)) {
    if (!DirEnt.is_regular_file()) continue;
    if (DirEnt.path().extension() != ".bstate") continue;
    auto FPath = DirEnt.path();

    EXPECT_EQ(RunTest(FPath), true);
  }
}

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

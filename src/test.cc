#include <iostream>
#include <filesystem>

#include <gtest/gtest.h>

#include "sim.hpp"

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
    std::ifstream testf{testf_path};
    if (!testf) {
      std::cerr << "ERROR: could not open test file <" << testf_path << ">\n";
      return false;
    }

    model.init(testf);
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

  bool TestAnsBstate(std::filesystem::path bstate_path) {
    std::filesystem::path ansf_path = bstate_path;
    ansf_path.replace_extension(".ans");

    model.init(bstate_path);
    if (!model.isValid()) {
      std::cerr << "ERROR: failed to initialize model correctly\n";
      std::cerr << bstate_path << '\n';
      return false;
    }

    model.execute();
    if (!model.isValid()) {
      std::cerr << "ERROR: model invalid after execution\n";
      std::cerr << bstate_path << '\n';
      return false;
    }

    ref_model.init(ansf_path);
    if (!model.isValid()) {
      std::cerr << "ERROR: failed to initialize ref model correctly\n";
      std::cerr << ansf_path << '\n';

      return false;
    }

    return ref_model == model;
  }

  bool TestAnsELF(std::filesystem::path elf_path) {
    std::filesystem::path ansf_path = elf_path;
    ansf_path.replace_extension(".ans");

    model = rv32i_sim::RVModel(elf_path);
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

    ref_model.init(ansf_path);
    if (!model.isValid()) {
      std::cerr << "ERROR: failed to initialize ref model correctly\n";
      std::cerr << ansf_path << '\n';

      return false;
    }

    return ref_model == model;
  }
};

TEST_F(TestRVModel, ADD) {
  std::filesystem::path test_dir = "../test/insn/add";
  for (auto const &dir_entry :
                      std::filesystem::directory_iterator(test_dir)) {
    if (!dir_entry.is_regular_file()) continue;
    if (dir_entry.path().extension() != ".bstate") continue;
    auto fpath = dir_entry.path();

    EXPECT_EQ(TestAnsBstate(fpath), true);
  }
}

TEST_F(TestRVModel, SUB) {
  std::filesystem::path test_dir = "../test/insn/sub";
  for (auto const &dir_entry :
                      std::filesystem::directory_iterator(test_dir)) {
    if (!dir_entry.is_regular_file()) continue;
    if (dir_entry.path().extension() != ".bstate") continue;
    auto fpath = dir_entry.path();

    EXPECT_EQ(TestAnsBstate(fpath), true);
  }
}

TEST_F(TestRVModel, SLL) {
  std::filesystem::path test_dir = "../test/insn/sll";
  for (auto const &dir_entry :
                      std::filesystem::directory_iterator(test_dir)) {
    if (!dir_entry.is_regular_file()) continue;
    if (dir_entry.path().extension() != ".bstate") continue;
    auto fpath = dir_entry.path();

    EXPECT_EQ(TestAnsBstate(fpath), true);
  }
}

TEST_F(TestRVModel, SLT) {
  std::filesystem::path test_dir = "../test/insn/slt";
  for (auto const &dir_entry :
                      std::filesystem::directory_iterator(test_dir)) {
    if (!dir_entry.is_regular_file()) continue;
    if (dir_entry.path().extension() != ".bstate") continue;
    auto fpath = dir_entry.path();

    EXPECT_EQ(TestAnsBstate(fpath), true);
  }
}

TEST_F(TestRVModel, SLTU) {
  std::filesystem::path test_dir = "../test/insn/sltu";
  for (auto const &dir_entry :
                      std::filesystem::directory_iterator(test_dir)) {
    if (!dir_entry.is_regular_file()) continue;
    if (dir_entry.path().extension() != ".bstate") continue;
    auto fpath = dir_entry.path();

    EXPECT_EQ(TestAnsBstate(fpath), true);
  }
}

TEST_F(TestRVModel, XOR) {
  std::filesystem::path test_dir = "../test/insn/xor";
  for (auto const &dir_entry :
                      std::filesystem::directory_iterator(test_dir)) {
    if (!dir_entry.is_regular_file()) continue;
    if (dir_entry.path().extension() != ".bstate") continue;
    auto fpath = dir_entry.path();

    EXPECT_EQ(TestAnsBstate(fpath), true);
  }
}

TEST_F(TestRVModel, SRA) {
  std::filesystem::path test_dir = "../test/insn/sra";
  for (auto const &dir_entry :
                      std::filesystem::directory_iterator(test_dir)) {
    if (!dir_entry.is_regular_file()) continue;
    if (dir_entry.path().extension() != ".bstate") continue;
    auto fpath = dir_entry.path();

    EXPECT_EQ(TestAnsBstate(fpath), true);
  }
}

TEST_F(TestRVModel, OR) {
  std::filesystem::path test_dir = "../test/insn/or";
  for (auto const &dir_entry :
                      std::filesystem::directory_iterator(test_dir)) {
    if (!dir_entry.is_regular_file()) continue;
    if (dir_entry.path().extension() != ".bstate") continue;
    auto fpath = dir_entry.path();

    EXPECT_EQ(TestAnsBstate(fpath), true);
  }
}

TEST_F(TestRVModel, AND) {
  std::filesystem::path test_dir = "../test/insn/or";
  for (auto const &dir_entry :
                      std::filesystem::directory_iterator(test_dir)) {
    if (!dir_entry.is_regular_file()) continue;
    if (dir_entry.path().extension() != ".bstate") continue;
    auto fpath = dir_entry.path();

    EXPECT_EQ(TestAnsBstate(fpath), true);
  }
}

TEST_F(TestRVModel, ELF_FILE) {
  std::filesystem::path test_dir = "../test/elf";
  for (auto const &dir_entry :
                      std::filesystem::directory_iterator(test_dir)) {
    if (!dir_entry.is_regular_file()) continue;
    if (dir_entry.path().extension() != ".elf") continue;
    auto fpath = dir_entry.path();

    EXPECT_EQ(TestAnsELF(fpath), true);
  }
}

TEST_F(TestRVModel, stress) {
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

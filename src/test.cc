#include <iostream>
#include <filesystem>

#include <gtest/gtest.h>

#include "sim.hpp"

class TestRVModel : public ::testing::Test {

protected:
  std::filesystem::path testf_path_;
  rv32i_sim::RVModel model;

  virtual void SetUp() {
    model = rv32i_sim::RVModel{};
  }

  virtual void TearDown() {}

  void RunTest(std::filesystem::path testf_path) {
    testf_path_ = testf_path;
    std::ifstream testf{testf_path_};
    if (!testf) {
      std::cerr << "ERROR: could not open test file <" << testf_path_ << ">\n";
      return;
    }

    model.init(testf);
    model.execute();
  }

  bool TestAns(std::filesystem::path testf_path) {
    testf_path_ = testf_path;
    std::ifstream testf{testf_path_};
    if (!testf) {
      std::cerr << "ERROR: could not open test file <" << testf_path_ << ">\n";
      return false;
    }

    std::filesystem::path ansf_path = testf_path;
    ansf_path.replace_extension(".ans");

    std::ifstream ansf{ansf_path};
    if (!ansf) {
      std::cerr << "ERROR: could not open answer file <" << ansf_path << ">\n";
      return false;
    }

    model.init(testf);
    model.execute();
    rv32i_sim::RVModel ref_model(ansf);

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

    EXPECT_EQ(TestAns(fpath), true);
  }
}

TEST_F(TestRVModel, SUB) {
  std::filesystem::path test_dir = "../test/insn/sub";
  for (auto const &dir_entry :
                      std::filesystem::directory_iterator(test_dir)) {
    if (!dir_entry.is_regular_file()) continue;
    if (dir_entry.path().extension() != ".bstate") continue;
    auto fpath = dir_entry.path();

    EXPECT_EQ(TestAns(fpath), true);
  }
}

TEST_F(TestRVModel, SLL) {
  std::filesystem::path test_dir = "../test/insn/sll";
  for (auto const &dir_entry :
                      std::filesystem::directory_iterator(test_dir)) {
    if (!dir_entry.is_regular_file()) continue;
    if (dir_entry.path().extension() != ".bstate") continue;
    auto fpath = dir_entry.path();

    EXPECT_EQ(TestAns(fpath), true);
  }
}

TEST_F(TestRVModel, SLT) {
  std::filesystem::path test_dir = "../test/insn/slt";
  for (auto const &dir_entry :
                      std::filesystem::directory_iterator(test_dir)) {
    if (!dir_entry.is_regular_file()) continue;
    if (dir_entry.path().extension() != ".bstate") continue;
    auto fpath = dir_entry.path();

    EXPECT_EQ(TestAns(fpath), true);
  }
}

TEST_F(TestRVModel, SLTU) {
  std::filesystem::path test_dir = "../test/insn/sltu";
  for (auto const &dir_entry :
                      std::filesystem::directory_iterator(test_dir)) {
    if (!dir_entry.is_regular_file()) continue;
    if (dir_entry.path().extension() != ".bstate") continue;
    auto fpath = dir_entry.path();

    EXPECT_EQ(TestAns(fpath), true);
  }
}

TEST_F(TestRVModel, XOR) {
  std::filesystem::path test_dir = "../test/insn/xor";
  for (auto const &dir_entry :
                      std::filesystem::directory_iterator(test_dir)) {
    if (!dir_entry.is_regular_file()) continue;
    if (dir_entry.path().extension() != ".bstate") continue;
    auto fpath = dir_entry.path();

    EXPECT_EQ(TestAns(fpath), true);
  }
}

TEST_F(TestRVModel, SRA) {
  std::filesystem::path test_dir = "../test/insn/sra";
  for (auto const &dir_entry :
                      std::filesystem::directory_iterator(test_dir)) {
    if (!dir_entry.is_regular_file()) continue;
    if (dir_entry.path().extension() != ".bstate") continue;
    auto fpath = dir_entry.path();

    EXPECT_EQ(TestAns(fpath), true);
  }
}

TEST_F(TestRVModel, OR) {
  std::filesystem::path test_dir = "../test/insn/or";
  for (auto const &dir_entry :
                      std::filesystem::directory_iterator(test_dir)) {
    if (!dir_entry.is_regular_file()) continue;
    if (dir_entry.path().extension() != ".bstate") continue;
    auto fpath = dir_entry.path();

    EXPECT_EQ(TestAns(fpath), true);
  }
}

TEST_F(TestRVModel, AND) {
  std::filesystem::path test_dir = "../test/insn/or";
  for (auto const &dir_entry :
                      std::filesystem::directory_iterator(test_dir)) {
    if (!dir_entry.is_regular_file()) continue;
    if (dir_entry.path().extension() != ".bstate") continue;
    auto fpath = dir_entry.path();

    EXPECT_EQ(TestAns(fpath), true);
  }
}

TEST_F(TestRVModel, stress) {
  std::filesystem::path test_dir = "../test/stress";
  for (auto const &dir_entry :
                      std::filesystem::recursive_directory_iterator(test_dir)) {
    if (!dir_entry.is_regular_file()) continue;
    if (dir_entry.path().extension() != ".bstate") continue;
    auto fpath = dir_entry.path();
    std::cerr << fpath << '\n';
    RunTest(fpath);
  }
}

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

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
  }
};

TEST_F(TestRVModel, stress) {
  std::filesystem::path test_dir = "../test";
  for (auto const &dir_entry :
                      std::filesystem::recursive_directory_iterator(test_dir)) {
    if (!dir_entry.is_regular_file()) continue;
    auto fpath = dir_entry.path();
    RunTest(fpath);
  }
}

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

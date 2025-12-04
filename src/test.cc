#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>

#include <gtest/gtest.h>
#include <iterator>
#include <unistd.h>

#include "sim.hpp"
#include "io.hpp"

class TestRVModel : public ::testing::Test {

protected:
  std::filesystem::path TestPath_;
  std::filesystem::path AnsPath_;
  rv32i_sim::RVModel M_;

  virtual void SetUp() {
    M_ = rv32i_sim::RVModel{};
  }

  bool LoadElf(std::filesystem::path &ElfPath) {
    M_ = rv32i_sim::RVModel(ElfPath, std::unique_ptr<BufferIO>(new BufferIO), 0);
    return M_.isValid();
  }

  bool LoadElf(std::filesystem::path &ElfPath, std::vector<uint8_t> &Input) {
    M_ = rv32i_sim::RVModel(ElfPath, std::unique_ptr<BufferIO>(new BufferIO), 0);
    std::vector<uint8_t> &ReadBuf = dynamic_cast<BufferIO &>(M_.io())
                                      .getReadBuf(STDIN_FILENO);
    ReadBuf = Input;
    return M_.isValid();
  }

  // Load elf w/ overwritten stdin
  void LoadElf(std::filesystem::path &ElfPath, std::filesystem::path InFpath) {
    std::ifstream InFile(InFpath, std::ios::binary);
    ASSERT_EQ(InFile.is_open(), true);
    std::vector<uint8_t> Input = std::vector<uint8_t>(
      std::istreambuf_iterator<char>(InFile),
      std::istreambuf_iterator<char>()
    );

    ASSERT_EQ(LoadElf(ElfPath, Input), true);
  }

  bool LoadTest(std::filesystem::path &ElfPath) {
    EXPECT_EQ(LoadElf(ElfPath), true);
    std::filesystem::path AnsPath = ElfPath;
    AnsPath.replace_extension(".ans");
    EXPECT_EQ(std::filesystem::exists(AnsPath), true);
    AnsPath_ = AnsPath;
    return true;
  }

  bool LoadTest(std::filesystem::path &ElfPath, std::filesystem::path &InPath) {
    EXPECT_EQ(std::filesystem::exists(InPath), true);

    LoadElf(ElfPath, InPath);
    std::filesystem::path AnsPath = InPath;
    AnsPath.replace_extension(".ans");
    EXPECT_EQ(std::filesystem::exists(AnsPath), true);
    AnsPath_ = AnsPath;
    return true;
  }

  virtual void TearDown() {}

  bool RunTest(std::filesystem::path TestPath) {
    M_ = rv32i_sim::RVModel(TestPath, std::unique_ptr<BufferIO>(new BufferIO), 0);
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

  void TestAnsELF(std::filesystem::path ElfPath) {
    bool IsInAnsMode = false; // .in <-> .ans mode?
    const auto &D = ElfPath.parent_path();
    for (const auto &DE :
          std::filesystem::directory_iterator(D)) {
      if(!DE.is_regular_file()) continue;

      // if test is in format .in <-> .ans
      // then look for .ans for each .in
      // check that model gives correct answers
      // and finish test

      if(DE.path().extension() == ".in") {
        IsInAnsMode = true;
        auto InPath = DE.path();
        LoadTest(ElfPath, InPath);
        std::ifstream AnsF(AnsPath_, std::ios::binary);
        ASSERT_EQ(AnsF.is_open(), true && "Answer File must open");
        auto BufAns = std::vector<uint8_t>(
          std::istreambuf_iterator<char>(AnsF),
          std::istreambuf_iterator<char>()
        );

        M_.execute();
        BufferIO *IO = dynamic_cast<BufferIO *>(&M_.io());
        ASSERT_NE(IO, nullptr);
        EXPECT_EQ(IO->getWriteBuf(STDOUT_FILENO), BufAns)
          << "for input " << InPath;
      }
    }

    if (IsInAnsMode) return;

    // if test is in format .elf <-> .ans
    // (i.e. inputs are hardcoded in elf)
    // then look for .ans for the one .elf
    // check that model gives correct answer
    // and finish test

    LoadTest(ElfPath);
    std::ifstream AnsF(AnsPath_, std::ios::binary);
    ASSERT_EQ(AnsF.is_open(), true && "Answer File must open");
    auto BufAns = std::vector<uint8_t>(
      std::istreambuf_iterator<char>(AnsF),
      std::istreambuf_iterator<char>()
    );

    M_.execute();
    BufferIO *IO = dynamic_cast<BufferIO *>(&M_.io());
    EXPECT_NE(IO, nullptr);
    EXPECT_EQ(IO->getWriteBuf(STDOUT_FILENO), BufAns);
  }

};

#define TEST_F_ELF(TestName, TestDir)                                 \
TEST_F(TestRVModel, TestName) {                                       \
  for (auto const &DirEnt :                                           \
                      std::filesystem::directory_iterator(TestDir)) { \
    if (!DirEnt.is_regular_file()) continue;                          \
    if (DirEnt.path().extension() != ".elf") continue;                \
    auto ElfPath = DirEnt.path();                                     \
    TestAnsELF(ElfPath);                                              \
  }                                                                   \
}

TEST_F_ELF(PLUS, "../test/elf/plus");
TEST_F_ELF(FACTORIAL, "../test/elf/factorial");
TEST_F_ELF(ECHO, "../test/elf/echo");
TEST_F_ELF(FPADD, "../test/elf/fp_vector_add");

#undef TEST_F_ELF

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

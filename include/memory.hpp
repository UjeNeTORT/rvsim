#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <cassert>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include <elfio/elfio.hpp>

#include "segment.hpp"

namespace rv32i_sim {

namespace elf = ELFIO;

constexpr uint32_t DEFAULT_ADDR_SPACE = 1 << 16;
constexpr uint32_t DEFAULT_STACK_SIZE = 1 << 12;
constexpr uint32_t ENV_SEG_SIZE = 1 << 6;
constexpr uint32_t DEFAULT_CANARY_SIZE = 1 << 8;

const uint32_t IALIGN = 4;

constexpr uint8_t STACK_CANARY_BYTE = 0xcc; // to make canaries visible
constexpr uint8_t ENV_CODE_BYTE = 0xee; // to make environment code visible

const std::string RV32I_MEMORY_STATE_SIGNATURE = "RV32I_MEM_STATE";

enum class Endianness { LITTLE, BIG, }; // big endian have not been supported yet
enum class ELFError : uint8_t {
  OK = 0, //< everything is ok
  CLASS = 1, //< wrong class
  ENC = 2, //< wrong encoding (endianness)
  FILE = 3, //< cannot open file
};

ELFError checkELF(elf::elfio& elf_reader);
ELFError checkELF(std::filesystem::path& elf_path);

/** memory model for the simulator
 *
 * endianness: little (default)
*/
class MemoryModel final {
  class Page;
  std::unordered_map<uint32_t, Page> mem_;
  std::vector<Segment> segments_;

  Endianness endian_ = Endianness::LITTLE;
  bool is_valid_ = false;

public:
  MemoryModel(bool valid) : is_valid_(valid) {}
  MemoryModel(Endianness endian = Endianness::LITTLE) : endian_(endian) {}
  MemoryModel(std::vector<uint8_t> mem, std::vector<Segment> segments, bool valid = true) :
      mem_(mem), segments_(segments), is_valid_(valid) {}

  static MemoryModel fromELF(elf::elfio& elf_reader);
  static MemoryModel fromELF(std::filesystem::path& elf_path);
  static MemoryModel fromBstate(std::filesystem::path& mem_path);
  static MemoryModel fromBstate(std::ifstream& mem_file);

  // sets up stack segment of size = stack_size with canary at the top
  // returns address where initial sp is placed - the bottom of the segment
  uint32_t setUpStack(uint32_t stack_size = DEFAULT_STACK_SIZE);
  uint32_t setUpEnvironment(uint32_t pc_main);

  /// @brief create a segment and push at the end of memory
  /// @param size size of segment requested (can be a little bigger due to alignment)
  /// @param rights RWX
  /// @param align starting address alignment
  /// @return memory address of pushed segment
  uint32_t pushSegment(uint32_t size, uint8_t rights, uint8_t align);

  /// @brief push requested segment at the end of memory
  /// @param seg Segment which is to be pushed
  /// @return memory address of pushed segment
  /// @warning DISCARDS ALIGNMENT as it is assumed that `seg.vaddr` is already aligned
  uint32_t pushSegment(Segment seg);

  bool checkRights(uint32_t addr, uint8_t rights) const;

  bool isValid() const;

  bool operator==(const MemoryModel& other) const;

  void copy(uint32_t addr, const void * src, uint32_t n);
  void set(uint32_t addr, uint8_t val, uint32_t n);

  uint8_t readByte(uint32_t addr) const;
  uint16_t readHalf(uint32_t addr) const;
  uint32_t readWord(uint32_t addr) const;

  void writeByte(uint32_t addr, uint8_t val);
  void writeHalf(uint32_t addr, uint16_t val);
  void writeWord(uint32_t addr, uint32_t val);

  void binaryDump(std::ofstream& fout) const;
  std::ostream& print(std::ostream& out) const;
  std::ostream& printSegments(std::ostream& out) const;

  uint32_t size() const;
};

class MemoryModel::Page final {
  char *Data_;
  uint32_t VAddr_;
  static const size_t Size_ = 1ULL << 12; // 4K
public:
  Page(uint32_t VAddr) : Data_(new char[Size_]), VAddr_(VAddr) {}
  Page(const Page &Other) : Page(0U) {
    std::memcpy(Data_, Other.Data_, Size_);
    VAddr_ = Other.VAddr_;
  }

  Page(Page &&Other) noexcept {
    Data_ = nullptr;
    std::swap(Data_, Other.Data_);
    VAddr_ = Other.VAddr_;
  }

  Page &operator=(const Page &Rhs) {
    if (this == &Rhs) return *this;
    std::memcpy(Data_, Rhs.Data_, Size_);
    VAddr_ = Rhs.VAddr_;
    return *this;
  }

  Page &operator=(Page &&Rhs) noexcept {
    std::swap(Data_, Rhs.Data_);
    std::swap(VAddr_, Rhs.VAddr_);
    return *this;
  }

  ~Page() { delete [] Data_; }

  template <typename T = uint32_t>
  T get(uint32_t VAddr) {
    assert(VAddr >= VAddr_ && "Cannot access data at addr < page begin addr!");
    assert(VAddr < VAddr_ + Size_ && "Cannot access data at addr > page end addr");

    T Res = *reinterpret_cast<T *>(Data_ + (VAddr - VAddr_)); // offset relative to a page start addr
    return Res;
  }

  // @param VAddr virtual address of the byte
  // @returns byte at address VAddr
  // unsafe - no bounds check
  char &operator[] (uint32_t VAddr) noexcept { return *(Data_ + (VAddr - VAddr_)); }
};

std::ostream& operator<<(std::ostream& out, MemoryModel& memory);

} // rv32i_sim

#endif // MEMORY_HPP

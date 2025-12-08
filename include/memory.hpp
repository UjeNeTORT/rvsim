#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <unordered_map>
#include <vector>

#include "elfio/elfio.hpp"

#include "segment.hpp"

namespace rv32i_sim {

namespace elf = ELFIO;

const     uint32_t DEFAULT_ADDR_SPACE = 0xffff'ffffU;
const     uint32_t DEFAULT_STACK_ADDR = 0x2b2a'9ac0U;
constexpr uint32_t DEFAULT_STACK_SIZE = 0x1000;
const     uint32_t DEFAULT_ENV_ADDR = 0x7fff'f000U;
constexpr uint32_t ENV_SEG_SIZE = 64U;
constexpr uint32_t DEFAULT_CANARY_SIZE = 256U;

const uint32_t IALIGN = 4;

constexpr uint8_t STACK_CANARY_BYTE = 0xcc;
constexpr uint8_t ENV_CODE_BYTE = 0xee;

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
  using PageTable_t = std::unordered_map<uint32_t, Page>;
  PageTable_t mem_;
  std::vector<Segment> segments_;

  Endianness endian_ = Endianness::LITTLE;
  bool is_valid_ = false;

public:
  MemoryModel(bool valid) : is_valid_(valid) {}
  MemoryModel(Endianness endian = Endianness::LITTLE) : endian_(endian) {}
  MemoryModel(const PageTable_t &mem, const std::vector<Segment> &segments, bool valid = true) :
      mem_(mem), segments_(segments), is_valid_(valid) {}
  MemoryModel(PageTable_t &&mem, std::vector<Segment> &&segments, bool valid = true) :
      mem_(mem), segments_(segments), is_valid_(valid) {}

  static MemoryModel fromELF(elf::elfio& elf_reader);
  static MemoryModel fromELF(std::filesystem::path& elf_path);
private:
  // if page does not exist - allocate, else - do nothing
  uint32_t preparePage(uint32_t Addr);
  uint32_t writeArgv(uint32_t ArgvAddr, const std::vector<std::string> &ArgvVec);

public:
  // @note does not guarantee that the page exists
  uint32_t getPageAddr(uint32_t Addr) const;

  // sets up stack segment of size = stack_size with canary at the top
  // returns address where initial sp is placed - the bottom of the segment
  uint32_t setUpStack(uint32_t StkSize = DEFAULT_STACK_SIZE);
  uint32_t setUpStack(const std::vector<std::string> &ProgArgv,
                      uint32_t StkSize = DEFAULT_STACK_SIZE);
  uint32_t setUpEnvironment(uint32_t MainPC);

  /// @brief create a segment and push at the end of memory
  /// @param Size size of segment requested (can be a little bigger due to alignment)
  /// @param Rights RWX
  /// @param Align starting address alignment
  /// @return memory address of pushed segment
  uint32_t pushSegment(uint32_t Size, uint8_t Rights, uint8_t Align);

  /// @brief push requested segment at the end of memory
  /// @param Seg Segment which is to be pushed
  /// @return memory address of pushed segment
  /// @warning DISCARDS ALIGNMENT as it is assumed that `seg.vaddr` is already aligned
  uint32_t pushSegment(Segment Seg);

  bool checkRights(uint32_t Addr, uint8_t Rights) const;

  bool isValid() const;

  bool operator==(const MemoryModel& other) const;

  void memCopy(uint32_t Addr, const void * Src, uint32_t N);
  void memCopy(void *Dst, uint32_t Addr, uint32_t N);
  void memSet(uint32_t Addr, uint8_t Val, uint32_t N);

  template<typename T> T get(uint32_t Addr);
  template<typename T> void set(uint32_t Addr, T Val);

  // safe only for access within one single page
  uint8_t &operator[](uint32_t Addr);

  uint8_t  readByte(uint32_t Addr);
  uint16_t readHalf(uint32_t Addr);
  uint32_t readWord(uint32_t Addr);

  void writeByte(uint32_t Addr, uint8_t Val);
  void writeHalf(uint32_t Addr, uint16_t Val);
  void writeWord(uint32_t Addr, uint32_t Val);

  std::ostream& print(std::ostream& out) const;
  std::ostream& printSegments(std::ostream& out) const;

  uint32_t size() const;
};

class MemoryModel::Page final {
  uint8_t *Data_;
  uint32_t VAddr_;
  static const size_t Size_ = 1ULL << 12; // 4K
public:
  Page() : Data_(new uint8_t[Size_]), VAddr_(0U) {}
  Page(uint32_t VAddr) : Data_(new uint8_t[Size_]), VAddr_(VAddr) {}
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
  T get(uint32_t Offset) {
    assert(Offset < Size_ && "Cannot access data at addr > page end addr");

    T Res = *reinterpret_cast<T *>(Data_ + Offset);
    return Res;
  }

  template <typename T = uint32_t>
  void set(uint32_t Offset, T Val) {
    assert(Offset < Size_ && "Cannot access data at addr > page end addr");

    *reinterpret_cast<T *>(Data_ + Offset) = Val;
  }

  // @param VAddr virtual address of the byte
  // @returns byte at address VAddr
  // unsafe - no bounds check
  uint8_t &operator[] (uint32_t VAddr) noexcept { return *(Data_ + (VAddr - VAddr_)); }
};

std::ostream& operator<<(std::ostream& out, MemoryModel& memory);

} // namespace rv32i_sim

#endif // MEMORY_HPP

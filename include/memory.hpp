#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <fstream>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include <elfio/elfio.hpp>

#include "encoding.hpp"
#include "segment.hpp"

namespace rv32i_sim {

namespace elf = ELFIO;

constexpr uint32_t DEFAULT_ADDR_SPACE = 1 << 16;
constexpr uint32_t DEFAULT_STACK_SIZE = 1 << 12;
constexpr uint32_t ENV_SEG_SIZE = 1 << 6;
constexpr uint32_t DEFAULT_CANARY_SIZE = 1 << 8;

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
  std::vector<byte_t> mem_ = std::vector<byte_t>(DEFAULT_ADDR_SPACE);
  std::vector<Segment> segments_;

  Endianness endian_ = Endianness::LITTLE;
  bool is_valid_ = false;

public:
  MemoryModel(bool valid) : is_valid_(valid) {}
  MemoryModel(Endianness endian = Endianness::LITTLE) : endian_(endian) {}
  MemoryModel(std::vector<byte_t> mem, std::vector<Segment> segments, bool valid = true) :
      mem_(mem), segments_(segments), is_valid_(valid) {}

  static MemoryModel fromELF(elf::elfio& elf_reader);
  static MemoryModel fromELF(std::filesystem::path& elf_path);
  static MemoryModel fromBstate(std::filesystem::path& mem_path);
  static MemoryModel fromBstate(std::ifstream& mem_file);

  // sets up stack segment of size = stack_size with canary at the top
  // returns address where initial sp is placed - the bottom of the segment
  addr_t setUpStack(uint32_t stack_size = DEFAULT_STACK_SIZE);
  addr_t setUpEnvironment(addr_t pc_main);

  /// @brief create a segment and push at the end of memory
  /// @param size size of segment requested (can be a little bigger due to alignment)
  /// @param rights RWX
  /// @param align starting address alignment
  /// @return memory address of pushed segment
  addr_t pushSegment(addr_t size, uint8_t rights, uint8_t align);

  /// @brief push requested segment at the end of memory
  /// @param seg Segment which is to be pushed
  /// @return memory address of pushed segment
  /// @warning DISCARDS ALIGNMENT as it is assumed that `seg.vaddr` is already aligned
  addr_t pushSegment(Segment seg);

  bool checkRights(addr_t addr, uint8_t rights) const;

  bool isValid() const;

  bool operator==(const MemoryModel& other) const;

  void set(addr_t addr, uint8_t val, uint32_t n);

  byte_t readByte(addr_t addr) const;
  half_t readHalf(addr_t addr) const;
  word_t readWord(addr_t addr) const;

  void writeByte(addr_t addr, byte_t val);
  void writeHalf(addr_t addr, half_t val);
  void writeWord(addr_t addr, word_t val);

  void binaryDump(std::ofstream& fout) const;
  std::ostream& print(std::ostream& out) const;
  std::ostream& printSegments(std::ostream& out) const;

  uint32_t size() const;
};

std::ostream& operator<<(std::ostream& out, MemoryModel& memory);

} // rv32i_sim

#endif // MEMORY_HPP

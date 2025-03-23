#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <fstream>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include <elfio/elfio.hpp>

#include "encoding.hpp"

namespace elf = ELFIO;

static uint32_t fileBytesLeft(std::ifstream& file) {
  if (!file) { return 0; }
  uint32_t curr_pos = file.tellg();
  file.seekg(0, std::ios::end);
  uint32_t end_pos = file.tellg();
  file.seekg(curr_pos, std::ios::beg);

  return end_pos - curr_pos;
}

namespace rv32i_sim {

constexpr std::size_t DEFAULT_ADDR_SPACE = 1 << 16;
const std::string RV32I_MEMORY_STATE_SIGNATURE = "RV32I_MEM_STATE";

enum class Endianness { LITTLE, BIG, }; // todo support big endian
enum class ELFError : uint8_t {
  OK = 0, //< everything is ok
  CLASS = 1, //< wrong class
  ENC = 2, //< wrong encoding (endianness)
  FILE = 3, //< cannot open file
};

ELFError checkELF(elf::elfio& elf_reader) {
  addr_t elf_class = elf_reader.get_class();
  addr_t elf_encoding = elf_reader.get_encoding();

  if (elf_class != elf::ELFCLASS32) {
    std::cerr << "ERROR: wrong ELF class: " << elf_class
              << "(" << elf::ELFCLASS32 << " expected)\n";

    return ELFError::CLASS;
  }

  if (elf_encoding != elf::ELFDATA2LSB) {
    std::cerr << "ERROR: wrong encoding, sorry, only Little Endian is supported now\n";

    return ELFError::ENC;
  }

  return ELFError::OK;
}

ELFError checkELF(std::filesystem::path& elf_path) {
  elf::elfio elf_reader;
  if (!elf_reader.load(elf_path)) {
    std::cerr << "ERROR: failed to load ELF " << elf_path << "\n";

    return ELFError::FILE;
  }

  return checkELF(elf_reader);
}


/** provides memory model for the simulator
 * model is abstract and represents continuous virtual memory
 *
 * endianness: little (default)
*/
class MemoryModel {
  std::vector<byte_t> mem_ = std::vector<byte_t>(DEFAULT_ADDR_SPACE);
  Endianness endian_ = Endianness::LITTLE;
  bool is_valid_ = false;

public:
  MemoryModel(bool valid) : is_valid_(valid) {}
  MemoryModel(Endianness endian = Endianness::LITTLE) : endian_(endian) {}
  MemoryModel(std::vector<byte_t> mem, bool valid = true) : mem_(mem), is_valid_(valid) {}

  static MemoryModel fromELF(elf::elfio& elf_reader) {
    if(checkELF(elf_reader) != ELFError::OK) { return MemoryModel(false); }

    uint32_t seg_vaddr = 0;
    uint32_t seg_memsz = 0;
    uint32_t seg_filesz = 0;

    std::vector<byte_t> memory (DEFAULT_ADDR_SPACE);

    for (auto seg = elf_reader.segments.begin(), seg_end = elf_reader.segments.end();
         seg != seg_end; ++seg) {

      seg_vaddr = seg->get()->get_virtual_address();
      seg_memsz = seg->get()->get_memory_size();
      seg_filesz = seg->get()->get_file_size();

      // resize if required
      if (memory.size() < seg_vaddr + seg_memsz)
        memory.resize(seg_vaddr + seg_memsz);

      // handle .bss section
      if (seg_memsz > seg_filesz) {
        std::memcpy(memory.data() + seg_vaddr, seg->get()->get_data(), seg_filesz);
        std::memset(memory.data() + seg_vaddr + seg_filesz, 0, seg_memsz - seg_filesz);
      } else {
        std::memcpy(memory.data() + seg_vaddr, seg->get()->get_data(), seg_memsz);
      }
    }

    return MemoryModel(memory, true);
  }

  static MemoryModel fromELF(std::filesystem::path& elf_path) {
    elf::elfio elf_reader;
    if (!elf_reader.load(elf_path)) {
      std::cerr << "ERROR: failed to load ELF " << elf_path << "\n";

      return MemoryModel(false);
    }

    if(checkELF(elf_reader) != ELFError::OK) { return MemoryModel(false); }

    return MemoryModel::fromELF(elf_reader);
  }

  static MemoryModel fromBstate(std::filesystem::path& mem_path) {
    std::ifstream mem_file(mem_path);
    if (!mem_file) {
      std::cerr << "ERROR: failed to open " << mem_path << "\n";
      return MemoryModel(false);
    }

    return MemoryModel::fromBstate(mem_file);
  }

  static MemoryModel fromBstate(std::ifstream& mem_file) {
    if (!mem_file) {
      std::cerr << "ERROR: wrong memory file\n";
      return MemoryModel(false);
    }

    std::string signature(RV32I_MEMORY_STATE_SIGNATURE.size(), ' ');
    mem_file.read(signature.data(), RV32I_MEMORY_STATE_SIGNATURE.size() + 1);

    if (signature != RV32I_MEMORY_STATE_SIGNATURE) {
      std::cerr << "ERROR: mem state file signature mismatch:\n"
                << "<" << signature << "> vs <" << RV32I_MEMORY_STATE_SIGNATURE <<">\n";
      return MemoryModel(false);
    }

    std::streamsize bytes_left = fileBytesLeft(mem_file);
    uint32_t memory_size = bytes_left > DEFAULT_ADDR_SPACE ? bytes_left :
                                                                DEFAULT_ADDR_SPACE;
    std::vector<byte_t> memory(memory_size);
    mem_file.read(std::bit_cast<char *>(memory.data()), bytes_left);

    return MemoryModel(memory, true);
  }

  bool isValid() const { return is_valid_; }

  bool operator==(const MemoryModel& other) const {
    std::cerr << mem_.size() << ' ' << other.mem_.size() << '\n';
    return endian_ == other.endian_ && mem_ == other.mem_;
  }

  byte_t readByte(addr_t addr) const {
    return mem_[addr];
  }

  half_t readHalf(addr_t addr) const {
    half_t res = 0;
    for (int i = sizeof(half_t) - 1; i >= 0; --i) {
      res <<= sizeof(byte_t) * BITS_BYTE;
      res |= half_t(mem_[addr + i]);
    }

    return res;
  }

  word_t readWord(addr_t addr) const {
    word_t res = 0;
    for (int i = sizeof(word_t) - 1; i >= 0; --i) {
      res <<= sizeof(byte_t) * BITS_BYTE;
      res |= word_t(mem_[addr + i]);
    }

    return res;
  }

  void writeByte(addr_t addr, byte_t val) {
    mem_[addr] = val;
  }

  void writeHalf(addr_t addr, half_t val) {
    for (int i = 0; i != sizeof(half_t); ++i) {
      byte_t curr = val & 0xFF;
      mem_[addr++] = curr;
      val >>= BITS_BYTE; // next byte
    }
  }

  void writeWord(addr_t addr, word_t val) {
    for (int i = 0; i != sizeof(word_t); ++i) {
      byte_t curr = val & 0xFF;
      mem_[addr++] = curr;
      val >>= BITS_BYTE; // next byte
    }
  }

  void binaryDump(std::ofstream& fout) const {
    fout.write(RV32I_MEMORY_STATE_SIGNATURE.c_str(),
               RV32I_MEMORY_STATE_SIGNATURE.size() + 1);
    // reinterpret:  byte_t * -> char *, and add const
    fout.write(reinterpret_cast<const char *>(mem_.data()), mem_.size());

    // as bstate files must be at least DEFAULT_ADDR_SPACE large
    // fill all the rest with zeros
    if (mem_.size() >= DEFAULT_ADDR_SPACE)
      return;

    uint32_t bytes_left = DEFAULT_ADDR_SPACE - mem_.size();
    std::vector<byte_t> null_vec(bytes_left);

    // reinterpret:  byte_t * -> char *, and add const
    fout.write(reinterpret_cast<const char *>(null_vec.data()), null_vec.size());
  }

  std::ostream& print(std::ostream& out) const {
    out << "Memory[" << mem_.size() << "] (examine with binaryDump)\n";
    // todo more verbose
    return out;
  }
};

std::ostream& operator<<(std::ostream& out, MemoryModel& memory) {
  memory.print(out);
  return out;
}

} // rv32i_sim

#endif // MEMORY_HPP

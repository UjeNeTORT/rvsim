#include "memory.hpp"

#include <bit>
#include <cassert>
#include <climits>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <string>

namespace elf = ELFIO;

static uint32_t fileBytesLeft(std::ifstream& file) {
  if (!file) { return 0; }
  uint32_t curr_pos = file.tellg();
  file.seekg(0, std::ios::end);
  uint32_t end_pos = file.tellg();
  file.seekg(curr_pos, std::ios::beg);

  return end_pos - curr_pos;
}

/// @brief resize vector to make its size % align == 0
/// @return number of new elements inserted
template <typename T>
static uint32_t alignAs(std::vector<T>& vec, uint32_t align) {
  uint32_t vec_size = vec.size();
  if (vec_size % align) return 0;

  uint32_t new_size = vec_size + (align - vec_size % align);
  vec.resize(new_size);
  return new_size;
}

namespace rv32i_sim {

ELFError checkELF(elf::elfio& elf_reader) {
  uint32_t elf_class = elf_reader.get_class();
  uint32_t elf_encoding = elf_reader.get_encoding();

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

MemoryModel MemoryModel::fromELF(elf::elfio& elf_reader) {
  if(checkELF(elf_reader) != ELFError::OK) { return MemoryModel(false); }

  uint32_t seg_vaddr = 0;
  uint32_t seg_memsz = 0;
  uint32_t seg_filesz = 0;
  uint32_t seg_rights = 0;
  uint32_t seg_align = 0;

  std::vector<uint8_t> memory (DEFAULT_ADDR_SPACE);
  std::vector<Segment> segments;

  auto seg = elf_reader.segments.begin();
  auto seg_end = elf_reader.segments.end();
  for ( ; seg != seg_end; ++seg) {
    seg_vaddr = seg->get()->get_virtual_address();
    seg_memsz = seg->get()->get_memory_size();
    seg_filesz = seg->get()->get_file_size();
    seg_rights = seg->get()->get_flags();
    seg_align = seg->get()->get_align();

    // resize if required
    if (memory.size() < seg_vaddr + seg_memsz)
      memory.resize(seg_vaddr + seg_memsz);

    // handle .bss section
    if (seg_memsz > seg_filesz) {
      std::memcpy(memory.data() + seg_vaddr, seg->get()->get_data(), seg_filesz);
      std::memset(memory.data() + seg_vaddr + seg_filesz, 0x00, seg_memsz - seg_filesz);
    } else {
      std::memcpy(memory.data() + seg_vaddr, seg->get()->get_data(), seg_memsz);
    }

    // create a segment for loaded data
    segments.push_back(
      Segment(seg_vaddr, seg_memsz, seg_rights, seg_align)
    );
  }

  return MemoryModel(memory, segments, true);
}

MemoryModel MemoryModel::fromELF(std::filesystem::path& elf_path) {
  elf::elfio elf_reader;
  if (!elf_reader.load(elf_path)) {
    std::cerr << "ERROR: failed to load ELF " << elf_path << "\n";

    return MemoryModel(false);
  }

  if(checkELF(elf_reader) != ELFError::OK) { return MemoryModel(false); }

  return MemoryModel::fromELF(elf_reader);
}

MemoryModel MemoryModel::fromBstate(std::filesystem::path& mem_path) {
  std::ifstream mem_file(mem_path);
  if (!mem_file) {
    std::cerr << "ERROR: failed to open " << mem_path << "\n";
    return MemoryModel(false);
  }

  return MemoryModel::fromBstate(mem_file);
}

MemoryModel MemoryModel::fromBstate(std::ifstream& mem_file) {
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

  uint32_t file_size = static_cast<uint32_t>(fileBytesLeft(mem_file));
  uint32_t memory_size = file_size > DEFAULT_ADDR_SPACE ? file_size :
                                                              DEFAULT_ADDR_SPACE;
  std::vector<uint8_t> memory(memory_size);
  mem_file.read(std::bit_cast<char *>(memory.data()), file_size);

  memory_size = alignAs(memory, DEFAULT_ALIGN);

  // bstate format memory consists of a single segment
  // which is RWX. This is done to not overload format with
  // unnessessary data, as bstate is mostly used for testing and
  // debugging.
  std::vector<Segment> segments {
    Segment {
      0,
      memory_size,
      RIGHTS_R | RIGHTS_W | RIGHTS_X,
      DEFAULT_ALIGN
    }
  };

  return MemoryModel(memory, segments, true);
}

// sets up stack segment of size = stack_size with canary at the top
// returns address where initial sp is placed - the bottom of the segment
uint32_t MemoryModel::setUpStack(uint32_t stack_size) {
  assert(stack_size < MAX_STACK_SIZE && "Stack size is too big!");

  // stack resides at the bottom of the address space
  // and is protected by a canary segment from both sides
  uint32_t canary_top_vaddr = alignAs(mem_, DEFAULT_ALIGN);
  uint32_t stack_vaddr = canary_top_vaddr + DEFAULT_CANARY_SIZE;
  Segment stack {
    stack_vaddr,
    stack_size,
    RIGHTS_R | RIGHTS_W,
  };

  mem_.resize(mem_.size() + 2 * DEFAULT_CANARY_SIZE + stack_size);
  std::memset(mem_.data() + canary_top_vaddr, STACK_CANARY_BYTE, DEFAULT_CANARY_SIZE);
  std::memset(mem_.data() + stack_vaddr + stack_size,
                                              STACK_CANARY_BYTE, DEFAULT_CANARY_SIZE);

  Segment canary_top {canary_top_vaddr, DEFAULT_CANARY_SIZE, 0 /* access forbidden */};
  Segment canary_bottom {
    canary_top_vaddr + DEFAULT_CANARY_SIZE + stack_size,
    DEFAULT_CANARY_SIZE,
    0 // access forbidden
  };

  segments_.push_back(canary_top);
  segments_.push_back(stack);
  segments_.push_back(canary_bottom);

  return stack_vaddr + stack_size - sizeof(uint32_t);
}

uint32_t MemoryModel::pushSegment(uint32_t size, uint8_t rights, uint8_t align) {
  uint32_t seg_vaddr = alignAs(mem_, align);

  if (mem_.size() < seg_vaddr + size)
    mem_.resize(seg_vaddr + size);

  std::memset(mem_.data() + seg_vaddr, ENV_CODE_BYTE, size);

  segments_.push_back(
    Segment {
      seg_vaddr,
      size,
      rights,
      align,
    }
  );

  return seg_vaddr;
}

uint32_t MemoryModel::pushSegment(Segment seg) {
  assert(seg.getVaddr() >= mem_.size() && "New segment cannot overlap the existing one");

  uint32_t max_addr = seg.getVaddr() + seg.getSize();
  if (mem_.size() < max_addr) mem_.resize(max_addr);

  std::memset(mem_.data() + seg.getVaddr(), ENV_CODE_BYTE, seg.getSize());

  segments_.push_back(seg);

  return max_addr;
}

bool MemoryModel::checkRights(uint32_t addr, uint8_t rights) const {
  for (auto seg : segments_) {
    if (seg.getVaddr() <= addr && addr < seg.getVaddr() + seg.getSize()) {
      return seg.checkRights(rights);
    }
  }

  return false;
}

bool MemoryModel::isValid() const {
  return is_valid_;
}

// comparison is so complicated to deal with the cases of vectors with
// non-meaningful zeros in the end
bool MemoryModel::operator==(const MemoryModel& other) const {
  bool mem_eq = mem_ == other.mem_;
  if (!mem_eq) {
    std::size_t size = mem_.size();
    std::size_t other_size = other.mem_.size();

    if (size < other_size) {
      mem_eq = std::equal(mem_.begin(), mem_.end(), other.mem_.begin());

      for (unsigned i = size; i < other_size; i++) {
        if (other.mem_[i] != 0x00) return false;
      }

      mem_eq = true;
    } else if (size > other_size) {
      mem_eq = std::equal(other.mem_.begin(), other.mem_.end(), mem_.begin());

      for (unsigned i = size; i < other_size; i++) {
        if (mem_[i] != 0x00) return false;
      }

      mem_eq = true;
    }
  }

  // todo maybe i can improve this
  return endian_ == other.endian_ && mem_eq;
}

void MemoryModel::set(uint32_t addr, uint8_t val, uint32_t n) {
  if (mem_.size() < addr + n) mem_.resize(addr + n);
  std::memset(mem_.data() + addr, val, n);
}

uint8_t MemoryModel::readByte(uint32_t addr) const {
  assert(checkRights(addr, RIGHTS_R) && "No rights to read");
  assert(addr % sizeof(uint32_t) == 0 && "Address not aligned");
  assert(addr < mem_.size() && "Address must be within bounds of loaded memory");
  return mem_[addr];
}

uint16_t MemoryModel::readHalf(uint32_t addr) const {
  assert(checkRights(addr, RIGHTS_R) && "No rights to read");
  assert(addr % sizeof(uint32_t) == 0 && "Address not aligned");
  assert(addr < mem_.size() && "Address must be within bounds of loaded memory");
  uint16_t res = 0;
  for (int i = sizeof(uint16_t) - 1; i >= 0; --i) {
    res <<= sizeof(uint8_t) * CHAR_BIT;
    res |= uint16_t(mem_[addr + i]);
  }

  return res;
}

uint32_t MemoryModel::readWord(uint32_t addr) const {
  assert(checkRights(addr, RIGHTS_R) && "No rights to read");
  assert(addr % sizeof(uint32_t) == 0 && "Address not aligned");
  assert(addr < mem_.size() && "Address must be within bounds of loaded memory");
  uint32_t res = 0;
  for (int i = sizeof(uint32_t) - 1; i >= 0; --i) {
    res <<= sizeof(uint8_t) * CHAR_BIT;
    res |= uint32_t(mem_[addr + i]);
  }

  return res;
}

void MemoryModel::writeByte(uint32_t addr, uint8_t val) {
  assert(checkRights(addr, RIGHTS_W) && "No rights to write");
  assert(addr < mem_.size() && "Address must be within bounds of loaded memory");
  mem_[addr] = val;
}

void MemoryModel::writeHalf(uint32_t addr, uint16_t val) {
  assert(checkRights(addr, RIGHTS_W) && "No rights to write");
  assert(addr < mem_.size() && "Address must be within bounds of loaded memory");
  for (int i = 0; i != sizeof(uint16_t); ++i) {
    uint8_t curr = val & 0xFF;
    mem_[addr++] = curr;
    val >>= CHAR_BIT; // next byte
  }
}

void MemoryModel::writeWord(uint32_t addr, uint32_t val) {
  assert(checkRights(addr, RIGHTS_W) && "No rights to write");
  assert(addr < mem_.size() && "Address must be within bounds of loaded memory");
  for (int i = 0; i != sizeof(uint32_t); ++i) {
    uint8_t curr = val & 0xFF;
    mem_[addr++] = curr;
    val >>= CHAR_BIT; // next byte
  }
}

void MemoryModel::binaryDump(std::ofstream& fout) const {
  fout.write(RV32I_MEMORY_STATE_SIGNATURE.c_str(),
              RV32I_MEMORY_STATE_SIGNATURE.size() + 1);
  // reinterpret:  uint8_t * -> char *, and add const
  fout.write(reinterpret_cast<const char *>(mem_.data()), mem_.size());

  // as bstate files must be at least DEFAULT_ADDR_SPACE large
  // fill all the rest with zeros
  if (mem_.size() >= DEFAULT_ADDR_SPACE)
    return;

  uint32_t bytes_left = DEFAULT_ADDR_SPACE - mem_.size();
  std::vector<uint8_t> null_vec(bytes_left);

  // reinterpret:  uint8_t * -> char *, and add const
  fout.write(reinterpret_cast<const char *>(null_vec.data()), null_vec.size());
}

std::ostream& MemoryModel::print(std::ostream& out) const {
  out << "Memory[" << mem_.size() << "] (examine with binaryDump)\n";
  // todo more verbose
  return out;
}

std::ostream& MemoryModel::printSegments(std::ostream& out) const {
  for (auto&& seg : segments_) {
    out << "Segment: " << seg.getVaddr() << " + " << seg.getSize() << " ("
        << +seg.getRights() << ")\n";
  }

  return out;
}

uint32_t MemoryModel::size() const {
  return mem_.size();
}

std::ostream& operator<<(std::ostream& out, MemoryModel& memory) {
  memory.print(out);
  return out;
}

} // rv32i_sim

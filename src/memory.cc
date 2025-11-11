#include "memory.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <vector>

#include "elfio/elfio.hpp"

namespace elf = ELFIO;

namespace {

uint32_t fileBytesLeft(std::ifstream& file) {
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
uint32_t alignAs(std::vector<T>& vec, uint32_t align) {
  uint32_t vec_size = vec.size();
  if (vec_size % align) return 0;

  uint32_t new_size = vec_size + (align - vec_size % align);
  vec.resize(new_size);
  return new_size;
}
} // namespace
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

  uint32_t SegVaddr = 0;
  uint32_t SegMemsz = 0;
  uint32_t SegFilesz = 0;
  uint32_t SegRights = 0;
  uint32_t SegAlign = 0;

  MemoryModel MemM(true);
  std::vector<Segment> segments;

  auto SegIt = elf_reader.segments.begin();
  auto SegEnd = elf_reader.segments.end();
  for ( ; SegIt != SegEnd; ++SegIt) {
    auto *Seg = SegIt->get();
    if (Seg->get_type() != elf::PT_LOAD) continue;

    SegVaddr  = Seg->get_virtual_address();
    SegMemsz  = Seg->get_memory_size();
    SegFilesz = Seg->get_file_size();
    SegRights = Seg->get_flags();
    SegAlign  = Seg->get_align();

    // handle .bss section
    if (SegMemsz > SegFilesz) {
      MemM.memCopy(SegVaddr, Seg->get_data(), SegFilesz);
      MemM.memSet(SegVaddr + SegFilesz, 0x00, SegMemsz - SegFilesz);
    } else {
      MemM.memCopy(SegVaddr, Seg->get_data(), SegMemsz);
    }

    // create a segment for loaded data
    MemM.pushSegment(
      Segment(SegVaddr, SegMemsz, SegRights, SegAlign)
    );
  }

  return MemM;
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

MemoryModel MemoryModel::fromBstate(std::ifstream& MemFile) {
  if (!MemFile) {
    std::cerr << "ERROR: wrong memory file\n";
    return MemoryModel(false);
  }

  std::string Signature(RV32I_MEMORY_STATE_SIGNATURE.size(), ' ');
  MemFile.read(Signature.data(), RV32I_MEMORY_STATE_SIGNATURE.size() + 1);

  if (Signature != RV32I_MEMORY_STATE_SIGNATURE) {
    std::cerr << "ERROR: mem state file signature mismatch:\n"
              << "<" << Signature << "> vs <" << RV32I_MEMORY_STATE_SIGNATURE <<">\n";
    return MemoryModel(false);
  }

  // everything after the signature is the memory dump
  uint32_t MemorySize = fileBytesLeft(MemFile);

  // std::vector<uint8_t> memory(memory_size);
  // mem_file.read(std::bit_cast<char *>(memory.data()), file_size);
  // memory_size = alignAs(memory, DEFAULT_ALIGN);

  MemoryModel MemM(true);

  // bstate format memory consists of a single segment
  // which is RWX. This is done to not overload format with
  // unnessessary data, as bstate is mostly used for testing and
  // debugging.
  MemM.pushSegment(Segment(0, MemorySize, RIGHTS_R | RIGHTS_W | RIGHTS_X, DEFAULT_ALIGN));

  return MemM;
}

uint32_t MemoryModel::preparePage(uint32_t Addr) {
  uint32_t PageAddr = getPageAddr(Addr);
  if (mem_.find(PageAddr) == mem_.end())
    mem_.emplace(PageAddr, Page(PageAddr));
  return PageAddr;
}

uint32_t MemoryModel::getPageAddr(uint32_t Addr) const {
  return Addr & ~0xFFF; // nullify an offset within the page
}

// sets up stack segment of size = StackSize with canaries
// returns address where initial sp is placed - the bottom of the segment
uint32_t MemoryModel::setUpStack(uint32_t StackSize) {
  assert(StackSize < MAX_STACK_SIZE && "Stack size is too big!");

  // stack is located in the end of the address space
  // and is protected by canary segments from both sides
  uint32_t CanaryTopVaddr = DEFAULT_STACK_ADDR;
  uint32_t StackVaddr = CanaryTopVaddr + DEFAULT_CANARY_SIZE;

  memSet(CanaryTopVaddr,         STACK_CANARY_BYTE, DEFAULT_CANARY_SIZE);
  memSet(StackVaddr + StackSize, STACK_CANARY_BYTE, DEFAULT_CANARY_SIZE);

  Segment CanaryStart { CanaryTopVaddr,         DEFAULT_CANARY_SIZE, 0 };
  Segment Stack       { StackVaddr,             StackSize,           RIGHTS_R | RIGHTS_W };
  Segment CanaryEnd   { StackVaddr + StackSize, DEFAULT_CANARY_SIZE, 0 };

  segments_.push_back(CanaryStart);
  segments_.push_back(Stack);
  segments_.push_back(CanaryEnd);

  return StackVaddr + StackSize - sizeof(uint32_t); // sp
}

uint32_t MemoryModel::pushSegment(Segment Seg) {
  uint32_t MaxAddr = Seg.getVaddr() + Seg.getSize();
  segments_.push_back(Seg);

  return MaxAddr;
}

bool MemoryModel::checkRights(uint32_t Addr, uint8_t Rights) const {
  for (auto Seg : segments_) {
    if (Seg.getVaddr() <= Addr && Addr < Seg.getVaddr() + Seg.getSize()) {
      return Seg.checkRights(Rights);
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
  // bool mem_eq = mem_ == other.mem_;
  // if (!mem_eq) {
    // std::size_t size = mem_.size();
    // std::size_t other_size = other.mem_.size();

    // if (size < other_size) {
      // mem_eq = std::equal(mem_.begin(), mem_.end(), other.mem_.begin());

      // for (unsigned i = size; i < other_size; i++) {
        // if (other.mem_[i] != 0x00) return false;
      // }

      // mem_eq = true;
    // } else if (size > other_size) {
      // mem_eq = std::equal(other.mem_.begin(), other.mem_.end(), mem_.begin());

      // for (unsigned i = size; i < other_size; i++) {
        // if (mem_[i] != 0x00) return false;
      // }

      // mem_eq = true;
    // }
  // }

  // todo maybe i can improve this
  // return endian_ == other.endian_ && mem_eq;
  return endian_ == other.endian_;
}

void MemoryModel::memCopy(uint32_t Addr, const void * Src, uint32_t N) {
  const uint8_t *CSrc = reinterpret_cast<const uint8_t *>(Src);
  for (size_t Idx = 0; Idx != N; ++Addr, ++Idx)
    set<uint8_t>(Addr, CSrc[Idx]);
}

void MemoryModel::memSet(uint32_t Addr, uint8_t Val, uint32_t N) {
  for (size_t Idx = 0; Idx != N; ++Addr, ++Idx)
    set<uint8_t>(Addr, Val);
}

template<typename T>
T MemoryModel::get(uint32_t Addr) {
  uint32_t PageAddr = preparePage(Addr);
  uint32_t Offset   = Addr - PageAddr;
  return mem_[PageAddr].get<T>(Offset);
}

template<typename T>
void MemoryModel::set(uint32_t Addr, T Val) {
  uint32_t PageAddr = preparePage(Addr);
  uint32_t Offset   = Addr - PageAddr;
  mem_[PageAddr].set<T>(Offset, Val);
}

uint8_t &MemoryModel::operator[](uint32_t Addr) {
  uint32_t PageAddr = preparePage(Addr);
  uint32_t Offset = Addr - PageAddr;
  return mem_[PageAddr][Offset];
}

uint8_t MemoryModel::readByte(uint32_t Addr) {
  assert(checkRights(Addr, RIGHTS_R) && "No rights to read");
  assert(Addr % sizeof(uint32_t) == 0 && "Address not aligned");
  return get<uint8_t>(Addr);
}

uint16_t MemoryModel::readHalf(uint32_t Addr) {
  assert(checkRights(Addr, RIGHTS_R) && "No rights to read");
  assert(Addr % sizeof(uint32_t) == 0 && "Address not aligned");
  return get<uint16_t>(Addr);
}

uint32_t MemoryModel::readWord(uint32_t Addr) {
  assert(checkRights(Addr, RIGHTS_R) && "No rights to read");
  assert(Addr % sizeof(uint32_t) == 0 && "Address not aligned");
  return get<uint32_t>(Addr);
}

void MemoryModel::writeByte(uint32_t Addr, uint8_t Val) {
  assert(checkRights(Addr, RIGHTS_W) && "No rights to write");
  set<uint8_t>(Addr, Val);
}

void MemoryModel::writeHalf(uint32_t Addr, uint16_t Val) {
  assert(checkRights(Addr, RIGHTS_W) && "No rights to write");
  set<uint16_t>(Addr, Val);
}

void MemoryModel::writeWord(uint32_t Addr, uint32_t Val) {
  assert(checkRights(Addr, RIGHTS_W) && "No rights to write");
  set<uint32_t>(Addr, Val);
}

void MemoryModel::binaryDump(std::ofstream& fout) const {
  fout.write(RV32I_MEMORY_STATE_SIGNATURE.c_str(),
              RV32I_MEMORY_STATE_SIGNATURE.size() + 1);
  // reinterpret:  uint8_t * -> char *, and add const
  #if 0
    fout.write(reinterpret_cast<const char *>(mem_.data()), mem_.size());
  #endif
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
  return DEFAULT_ADDR_SPACE;
}

std::ostream& operator<<(std::ostream& out, MemoryModel& memory) {
  memory.print(out);
  return out;
}

} // rv32i_sim

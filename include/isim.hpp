#ifndef ISIM_HPP
#define ISIM_HPP

#include <iostream>

#include "encoding.hpp"
#include "memory.hpp"
#include "register_file.hpp"

namespace rv32i_sim {

class IRVModel  {
public:
  virtual void init(std::ifstream& model_state_file) = 0;
  virtual void init(const MemoryModel& mem_init, const RegisterFile& regs_init,
                                                                      addr_t pc_init) = 0;
  virtual void init(MemoryModel&& mem_init, RegisterFile&& regs_init, addr_t pc_init) = 0;

  virtual addr_t getPC() const = 0;
  virtual void setPC(addr_t pc_new) = 0;
  virtual ~IRVModel() = default;

  virtual byte_t readByte(addr_t addr) = 0;
  virtual half_t readHalf(addr_t addr) = 0;
  virtual word_t readWord(addr_t addr) = 0;

#ifdef RVBITS64
  virtual dword_t readDWord(addr_t addr) = 0;
#endif // RVBITS64

  virtual void writeByte(addr_t addr, byte_t val) = 0;
  virtual void writeHalf(addr_t addr, half_t val) = 0;
  virtual void writeWord(addr_t addr, word_t val) = 0;

#ifdef RVBITS64
  virtual void writeDWord(addr_t addr, dword_t val) = 0;
#endif // RVBITS64

  virtual addr_t getReg(Register reg) const = 0;
  virtual void setReg(Register reg, word_t val) = 0;

public:
  virtual void execute() = 0;
  virtual std::ostream& print(std::ostream& out) = 0;
  virtual void binaryDump(std::ofstream& fout) = 0;
};

std::ostream& operator<<(std::ostream& out, IRVModel& model) {
  model.print(out);
  return out;
}

} // rv32i_sim

#endif // ISIM_HPP

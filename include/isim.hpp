#ifndef ISIM_HPP
#define ISIM_HPP

#include <iostream>

#include "memory.hpp"
#include "register_file.hpp"

namespace rv32i_sim {

class IRVModel  {
public:
  virtual void init(std::ifstream& model_state_file) = 0;
  virtual void init(const MemoryModel& mem_init, const RegisterFile& regs_init,
                                                                      uint32_t pc_init) = 0;
  virtual void init(MemoryModel&& mem_init, RegisterFile&& regs_init, uint32_t pc_init) = 0;

  virtual bool isValid() const = 0;
  virtual uint32_t getPC() const = 0;
  virtual void setPC(uint32_t pc_new) = 0;

  virtual uint8_t readByte(uint32_t addr) = 0;
  virtual uint16_t readHalf(uint32_t addr) = 0;
  virtual uint32_t readWord(uint32_t addr) = 0;

  virtual void writeByte(uint32_t addr, uint8_t val) = 0;
  virtual void writeHalf(uint32_t addr, uint16_t val) = 0;
  virtual void writeWord(uint32_t addr, uint32_t val) = 0;

  virtual uint32_t getReg(Register reg) const = 0;
  virtual void setReg(Register reg, uint32_t val) = 0;

  virtual void execute() = 0;
  virtual void envCall() = 0;
  virtual void exit() = 0;

  virtual std::ostream& print(std::ostream& out) = 0;
  virtual void binaryDump(std::ofstream& fout) = 0;

  virtual ~IRVModel() = default;
};

std::ostream& operator<<(std::ostream& out, IRVModel& model) {
  model.print(out);
  return out;
}

} // rv32i_sim

#endif // ISIM_HPP

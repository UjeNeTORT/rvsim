#ifndef ISIM_HPP
#define ISIM_HPP

#include <cstdint>
#include <iostream>

#include "io.hpp"
#include "registers.hpp"

namespace rv32i_sim {

class IRVModel  {
public:
  virtual bool isValid() const = 0;
  virtual uint32_t getPC() const = 0;
  virtual void setPC(uint32_t pc_new) = 0;

  virtual uint8_t readByte(uint32_t addr) = 0;
  virtual uint16_t readHalf(uint32_t addr) = 0;
  virtual uint32_t readWord(uint32_t addr) = 0;
  virtual void memCopy(uint32_t Addr, const void * Src, uint32_t N) = 0;
  virtual void memCopy(void * Dst, uint32_t Addr, uint32_t N) = 0;

  virtual void writeByte(uint32_t addr, uint8_t val) = 0;
  virtual void writeHalf(uint32_t addr, uint16_t val) = 0;
  virtual void writeWord(uint32_t addr, uint32_t val) = 0;

  virtual uint32_t getReg(Register reg) const = 0;
  virtual void setReg(Register reg, uint32_t val) = 0;
  virtual const IOInterface &io() = 0;

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

} // namespace rv32i_sim

#endif // ISIM_HPP

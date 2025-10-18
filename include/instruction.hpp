#ifndef INSTRUCTION_HPP
#define INSTRUCTION_HPP

#include <bitset>
#include <cassert>
#include <variant>

class IRVModel;

namespace RVISA {

enum class RVInsnTypes : uint32_t;

class IRVInsn {
public:
  virtual uint32_t getOpcode() const = 0;
	virtual RVInsnTypes getType() const = 0;

	// returns index of the pushed operand
	virtual uint32_t addOperand(uint32_t OpVal, std::string Name) = 0;
	virtual uint32_t getOperand(uint32_t OpIdx) const = 0;
	virtual uint32_t nOperands() const = 0;

	virtual std::string getName() const = 0;

	virtual void execute(rv32i_sim::IRVModel &Model) const = 0;
	virtual void print(std::ostream &Out) const = 0;

	virtual ~IRVInsn() = default;
};

std::ostream &operator<<(std::ostream &Out, const IRVInsn &Insn) {
  Insn.print(Out);
  return Out;
}

} // RVISA

#endif // INSTRUCTION_HPP

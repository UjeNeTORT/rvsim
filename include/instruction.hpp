#ifndef INSTRUCTION_HPP
#define INSTRUCTION_HPP

#include <bitset>
#include <cassert>
#include <cstdint>

class IRVModel;

namespace RVISA {

enum class RVInsnTypes : uint32_t;

class IRVInsn {
public:
  virtual uint32_t getOpcode() const = 0;
	virtual RVInsnTypes getType() const = 0;
	virtual std::string getName() const = 0;

	// @returns index of the pushed operand
	virtual void setOperand(uint32_t OpIdx, uint32_t OpVal) = 0;
	virtual uint32_t getOperand(uint32_t OpIdx) const = 0;
	virtual uint32_t nOperands() const = 0;

	// @brief encode operands
	// @param Operands[0] - operand in most significant bits
	// @param Operands[last] - opnd in least significant bits
	// @throws `std::out_of_range` exception if `Operands` vector is too small,
	// 					if it is too big, the extra elements are ignored
	// @returns freshly encoded instruction
	virtual uint32_t encode(std::vector<uint32_t> Operands) = 0;

	// @brief update instruction by encoding different operands
	virtual void encode(uint32_t NewOpcode) = 0;

	virtual void execute(rv32i_sim::IRVModel &Model) const = 0;
	virtual void print(std::ostream &Out) const = 0;

	virtual ~IRVInsn() = default;
};

std::ostream &operator<<(std::ostream &Out, const IRVInsn &Insn) {
  Insn.print(Out);
  return Out;
}

} // namespace RVISA

#endif // INSTRUCTION_HPP

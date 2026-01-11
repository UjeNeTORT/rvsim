#ifndef IDECODER_HPP
#define IDECODER_HPP

#include "instruction.hpp"

#include <cstdint>
#include <memory>

namespace rv32i_sim {

class IDecoder {
public:
  virtual std::shared_ptr<RVISA::IRVInsn> decode(uint32_t Opcode) = 0;
  virtual ~IDecoder() = default;
};

} // namespace rv32i_sim

#endif // IDECODER_HPP

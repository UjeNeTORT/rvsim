#ifndef EXEC_ENV_HPP
#define EXEC_ENV_HPP

#include "sim.hpp"
#include <cstdint>
#include <optional>
#include <unordered_map>

using namespace rv32i_sim;

enum EESyscall : uint32_t {
  READ  = 63,
  WRITE = 64,
  EXIT  = 93,
};

class EcallHandlersRegistry final {
  using EcallHandler = std::function<int(RVModel &Model)>;
  std::unordered_map<EESyscall, EcallHandler> handlers_;
public:
  void registerHandler(EESyscall Syscall, EcallHandler HandlerF) {
    handlers_[Syscall] = HandlerF;
  }
};

class ExecEnv final {
  EcallHandlersRegistry EcallHanders_;
  RVModel &Model_;
public:
  ExecEnv(RVModel &Model) : Model_(Model) {
    EcallHanders_.registerHandler(EESyscall::READ, [](RVModel &Model) -> int {});
    EcallHanders_.registerHandler(EESyscall::WRITE, [](RVModel &Model) -> int {});
    EcallHanders_.registerHandler(EESyscall::EXIT, [](RVModel &Model) -> int {});
  }

  void ecall();
};

#define MODEL_LOG if(logs_) std::cerr

static int ecallRead(RVModel &Model) {
  assert(Model.isValid());
  uint32_t Fd     = Model.getReg(Register::A0);
  uint32_t UsrBuf = Model.getReg(Register::A1);
  uint32_t Count  = Model.getReg(Register::A2);

  MODEL_LOG << "      a0 = " << Fd     << "\n"
            << "      a1 = " << UsrBuf << "\n"
            << "      a2 = " << Count  << "\n";

  if (Fd == 0 /*stdin*/) {
    uint8_t *Buffer = new uint8_t[Count];
    uint32_t Res = read(0, Buffer, Count);
    Model.setReg(Register::A0, Res);
    Model.mem_.memCopy(UsrBuf, Buffer, Count);
    delete [] Buffer;
  } else {
    MODEL_LOG << "ecall read is supported only for stdin (0), received: " << Fd << "\n";
  }
  Model.setPC(Model.getPC() + sizeof(uint32_t));

}

#undef MODEL_LOG

#endif // EXEC_ENV_HPP

#ifndef EXEC_ENV_HPP
#define EXEC_ENV_HPP

#include <cstdint>
#include <unordered_map>

#include "spdlog/common.h"
#include "spdlog/spdlog.h"

#include "isim.hpp"

using namespace rv32i_sim;

enum EESyscall : uint32_t {
  READ  = 63,
  WRITE = 64,
  EXIT  = 93,
};

static int ecallRead(IRVModel &Model) {
  assert(Model.isValid());
  uint32_t Fd     = Model.getReg(Register::A0);
  uint32_t UsrBuf = Model.getReg(Register::A1);
  uint32_t Count  = Model.getReg(Register::A2);

  SPDLOG_INFO("ecall \"read\" (a0 = {}, a1 = {}, a2 = {})",
    Fd, UsrBuf, Count);

  uint32_t Res = -1;
  if (Fd == 0 /*stdin*/) {
    uint8_t *Buffer = new uint8_t[Count];
    Res = read(0, Buffer, Count);
    Model.setReg(Register::A0, Res);
    Model.memCopy(UsrBuf, Buffer, Count);
    delete [] Buffer;
  } else {
    SPDLOG_ERROR("ecall read is supported only for stdin (0), received: {}", Fd);
  }
  Model.setPC(Model.getPC() + sizeof(uint32_t));

  return Res;
}

static int ecallWrite(IRVModel &Model) {
  assert(Model.isValid());
  uint32_t Fd     = Model.getReg(Register::A0);
  uint32_t UsrBuf = Model.getReg(Register::A1);
  uint32_t Count  = Model.getReg(Register::A2);

  SPDLOG_INFO("ecall \"write\" (a0 = {}, a1 = {}, a2 = {})",
    Fd, UsrBuf, Count);

  uint32_t Res = -1;
  if (Fd == 1 || Fd == 2) {
    uint8_t *Buffer = new uint8_t[Count];
    Model.memCopy(Buffer, UsrBuf, Count);
    Res = write(Fd, Buffer, Count);
    Model.setReg(Register::A0, Res);
    delete [] Buffer;
  } else {
    SPDLOG_ERROR("ecall write is supported only for stdout (1) and stderr (2), received: {}", Fd);
  }
  Model.setPC(Model.getPC() + sizeof(uint32_t));
  return Res;
}

static int ecallExit(IRVModel &Model) {
  assert(Model.isValid());
  uint32_t ExitCode = Model.getReg(Register::A0);

  SPDLOG_INFO("ecall \"exit\" (a0 = {})", ExitCode);
  Model.exit();
  return ExitCode;
}


class ExecEnv final {
  class EcallHandlersRegistry final {
    using EcallHandler = std::function<int(IRVModel &Model)>;
    std::unordered_map<EESyscall, EcallHandler> handlers_;
  public:
    void registerHandler(EESyscall Syscall, EcallHandler HndF) {
      handlers_[Syscall] = HndF;
    }
    EcallHandler get(EESyscall Syscall) { return handlers_[Syscall]; }
  };

  EcallHandlersRegistry EcallHanders_;

public:
  ExecEnv() {
    EcallHanders_.registerHandler(EESyscall::READ,  ecallRead);
    EcallHanders_.registerHandler(EESyscall::WRITE, ecallWrite);
    EcallHanders_.registerHandler(EESyscall::EXIT,  ecallExit);
  }

  void ecall(EESyscall Syscall, IRVModel &Model) {
    auto &&Hnd = EcallHanders_.get(Syscall);
    Hnd(Model);
  }
};

#endif // EXEC_ENV_HPP

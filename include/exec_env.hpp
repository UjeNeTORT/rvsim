#ifndef EXEC_ENV_HPP
#define EXEC_ENV_HPP

#include <cstdint>

enum class EESyscall : uint32_t {
  READ  = 63,
  WRITE = 64,
  EXIT  = 93,
};

#endif // EXEC_ENV_HPP

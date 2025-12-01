#pragma once

#include <cstdint>
#include <unistd.h>

class IOInterface {
public:
  virtual ssize_t read(int Fd, void *Data, uint32_t N) const = 0;
  virtual ssize_t write(int Fd, const void *Data, uint32_t N) const = 0;
  virtual ~IOInterface() = default;
};

class HostIO final : public IOInterface {
public:
  ssize_t read(int Fd, void *Data, uint32_t N) const override {
    return ::read(Fd, Data, N);
  }

  ssize_t write(int Fd, const void *Data, uint32_t N) const override {
    return ::write(Fd, Data, N);
  }
};

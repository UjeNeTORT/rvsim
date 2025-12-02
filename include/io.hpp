#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <unistd.h>
#include <unordered_map>
#include <vector>

class IOInterface {
public:
  virtual ssize_t read(int Fd, void *Data, uint32_t N) = 0;
  virtual ssize_t write(int Fd, const void *Data, uint32_t N) = 0;
  virtual ~IOInterface() = default;
};

class HostIO final : public IOInterface {
public:
  ssize_t read(int Fd, void *Data, uint32_t N) override {
    return ::read(Fd, Data, N);
  }

  ssize_t write(int Fd, const void *Data, uint32_t N) override {
    return ::write(Fd, Data, N);
  }
};

class BufferIO final : public IOInterface {
  // buffers which are 'read' to
  // Model.ecall read (InB [Fd])
  // they are called In, because they are imported (are set manually)
  // to give inputs for an interpreter on read ecall
  std::unordered_map<int, std::vector<uint8_t>> InB_;

  // buffer which are 'written' from
  // Model.ecall write (OutS [Fd])
  // they are called Out, because they are to be exported
  // to examine what have interpreter written w/ write ecall
  std::unordered_map<int, std::vector<uint8_t>> OutB_;
public:
  BufferIO() {
    addReadFd(STDIN_FILENO);
    addWriteFd(STDOUT_FILENO);
    addWriteFd(STDERR_FILENO);
  }

  ssize_t read(int Fd, void *Data, uint32_t N) override {
    assert(Data && "Data must be a non null ptr");
    const auto It = InB_.find(Fd);
    if (It == InB_.end()) return -1; // no such file descriptor (Fd)

    std::vector<uint8_t> &B = It->second;
    size_t OldSize = B.size();
    B.resize(OldSize + N);
    std::memcpy(Data, B.data() + OldSize, N);
    return N;
  }

  ssize_t write(int Fd, const void *Data, uint32_t N) override {
    assert(Data && "Data must be a non null ptr");
    const auto It = OutB_.find(Fd);
    if (It == OutB_.end()) return -1; // no such file descriptor (Fd)

    std::vector<uint8_t> &B = It->second;
    size_t OldSize = B.size();
    B.resize(OldSize + N);
    std::memcpy(B.data() + OldSize, Data, N);
    return N;
  }

  void addReadFd(int Fd) {
    InB_.insert(std::pair<int, std::vector<uint8_t>>(Fd, {}));
  }

  void addWriteFd(int Fd) {
    OutB_.insert(std::pair<int, std::vector<uint8_t>>(Fd, {}));
  }

  bool canReadFrom(int Fd) { return InB_.find(Fd)  != InB_.end(); }
  bool canWriteTo(int Fd)  { return OutB_.find(Fd) != InB_.end(); }

  // unsafe, Fd must be present in map
  // use canWriteTo() to check
  const std::vector<uint8_t> &getWriteBuf(int Fd) {
    return OutB_[Fd];
  }

  // unsafe, Fd must be present in map
  // use canReadFrom() to check
  std::vector<uint8_t> &getReadBuf(int Fd) {
    return InB_[Fd];
  }

};

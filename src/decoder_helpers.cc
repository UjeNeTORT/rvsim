#include <cassert>
#include <bit>
#include <cstdint>
#include <vector>
#include <string>

#include "decoder_helpers.hpp"

namespace RVDecoder {
int32_t sign_extend_8_to_32(uint8_t val) {
  return std::bit_cast<int32_t>(uint32_t(val) << 24) >> 24;
}

int32_t sign_extend_12_to_32(uint16_t val) {
  return std::bit_cast<int32_t>(uint32_t(val) << 20) >> 20;
}

int32_t sign_extend_13_to_32(uint16_t val) {
  return std::bit_cast<int32_t>(uint32_t(val) << 19) >> 19;
}

int32_t sign_extend_16_to_32(uint16_t val) {
  return std::bit_cast<int32_t>(uint32_t(val) << 16) >> 16;
}

int32_t sign_extend_21_to_32(uint32_t val) {
  return std::bit_cast<int32_t>(uint32_t(val) << 11) >> 11;
}

int32_t sign_extend_32_to_32(uint32_t val) {
  return std::bit_cast<int32_t>(val);
}

std::vector<uint32_t> createJalArgs(uint32_t Offset, uint32_t Rd) {
  std::vector<uint32_t> Res;
  Res.push_back(Offset & (1 << 20) >> 20);      // imm[20]
  Res.push_back(Offset & ((1 << 11) - 1) >> 1); // imm[10:1]
  Res.push_back(Offset & (1 << 11) >> 11);      // imm[11]
  Res.push_back(Offset & ((1 << 9) - 1) >> 12); // imm[19:12]
  Res.push_back(Rd);
  return Res;
}

std::pair<uint32_t, uint32_t> getJalArgs(std::vector<std::pair<uint32_t, std::string>> Encods) {
  assert(Encods.size() == 5);
  uint32_t Offset = 0;
  Offset |= Encods[1].first << 1;
  Offset |= Encods[2].first << 11;
  Offset |= Encods[3].first << 12;
  Offset |= Encods[0].first << 20;
  return std::pair<uint32_t, uint32_t>(Offset, Encods[4].first);
}
} // namespace RVDecoder

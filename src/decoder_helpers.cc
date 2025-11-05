#include <bit>
#include <cassert>
#include <cstdint>
#include <string>
#include <tuple>
#include <vector>

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
  Res.push_back((Offset >> 20) & 1);     // imm[20]
  Res.push_back((Offset >> 1)  & 0x3ff); // imm[10:1]
  Res.push_back((Offset >> 11) & 1);     // imm[11]
  Res.push_back((Offset >> 12) & 0xff);  // imm[19:12]
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

std::vector<uint32_t> createBArgs(uint32_t Imm, uint32_t Rs2, uint32_t Rs1) {
  std::vector<uint32_t> Res;
  Res.push_back((Imm >> 12) & 1);   // imm[12]
  Res.push_back((Imm >> 5) & 0x3f); // imm[10:5]
  Res.push_back(Rs2);
  Res.push_back(Rs1);
  Res.push_back((Imm >> 1) & 0xf);  // imm[4:1]
  Res.push_back((Imm >> 11) & 1);   // imm[11]
  return Res;
}

std::tuple<uint32_t, uint32_t, uint32_t> getBArgs(std::vector<std::pair<uint32_t, std::string>> Encods) {
  assert(Encods.size() == 6);
  uint32_t Imm = 0;
  Imm |= (Encods[0].first << 12) & 1;
  Imm |= (Encods[1].first << 5)  & 0x3f;
  Imm |= (Encods[4].first << 1)  & 0xf;
  Imm |= (Encods[5].first << 11) & 1;
  return std::tuple<uint32_t, uint32_t, uint32_t>(
    Imm, Encods[2].first /*Rs2*/, Encods[3].first /*Rs1*/
  );
}

uint32_t getSTOREImm(std::vector<std::pair<uint32_t, std::string>> Encods) {
  assert(Encods.size() == 4);
  uint32_t Imm = (Encods[0].first & 0x7f) << 5;
  Imm |= Encods[3].first & 0x1f;
  return Imm;
}

} // namespace RVDecoder

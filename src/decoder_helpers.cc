#include <bit>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cfloat>
#include <spdlog/spdlog.h>
#include <string>
#include <tuple>
#include <vector>

#include "decoder_helpers.hpp"

static void setBit(uint32_t &Val, uint32_t Pos, bool Pred) {
  if (Pred) Val |= Pred << Pos; // set
  else Val &= ~(1 << Pos); // unset
  return;
}

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

uint32_t zero_extend_16_to_32(uint16_t val) {
  return (uint32_t(val) << 16) >> 16;
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

std::pair<uint32_t, uint32_t> getJalArgs(std::vector<uint32_t> Encods) {
  assert(Encods.size() == 5);
  uint32_t Offset = 0;
  Offset |= Encods[1] << 1;
  Offset |= Encods[2] << 11;
  Offset |= Encods[3] << 12;
  Offset |= Encods[0] << 20;
  return std::pair<uint32_t, uint32_t>(Offset, Encods[4]);
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

std::tuple<uint32_t, uint32_t, uint32_t> getBArgs(std::vector<uint32_t> Encods) {
  assert(Encods.size() == 6);
  uint32_t Imm = 0;
  Imm |= (Encods[0] & 1) << 12;
  Imm |= (Encods[1] & 0x3f) << 5;
  Imm |= (Encods[4] & 0xf) << 1;
  Imm |= (Encods[5] & 1) << 11;
  return std::tuple<uint32_t, uint32_t, uint32_t>(
    Imm, Encods[2] /*Rs2*/, Encods[3] /*Rs1*/
  );
}

uint32_t getSTOREImm(std::vector<uint32_t> Encods) {
  assert(Encods.size() == 4);
  uint32_t Imm = (Encods[0] & 0x7f) << 5;
  Imm |= Encods[3] & 0x1f;
  return Imm;
}

uint32_t classifyS(float Op) {
  uint32_t Res = 0;
  switch(std::fpclassify(Op)) {
    case FP_INFINITE:
      setBit(Res, 0, Op < 0); // -inf
      setBit(Res, 7, Op > 0); // +inf
      break;
    case FP_NORMAL:
      setBit(Res, 1, Op < 0); // neg normal
      setBit(Res, 6, Op > 0); // pos normal
      break;
    case FP_SUBNORMAL:
      setBit(Res, 2, Op < 0); // neg subnormal
      setBit(Res, 5, Op > 0); // pos subnormal
      break;
    case FP_ZERO:
      setBit(Res, 3, Op < 0); // -0
      setBit(Res, 4, Op > 0); // +0
      break;
    case FP_NAN:
      setBit(Res, 8, 1); // signaling NaN
      setBit(Res, 9, 1); // quiet NaN
      break;
    default: break;
  }
  return Res;
}

float floatDivide(float Op1, float Op2) {
  float result = 0.0f;
  if (std::isnan(Op1) || std::isnan(Op2)) {
      result = std::numeric_limits<float>::quiet_NaN();
  } else if (std::isinf(Op1) && std::isinf(Op2)) {
      result = std::numeric_limits<float>::quiet_NaN();
  } else if (Op2 == 0.0f) {
      if (Op1 == 0.0f) {
          result = std::numeric_limits<float>::quiet_NaN();
      } else {
          bool sign = std::signbit(Op1) ^ std::signbit(Op2);
          result = std::copysign(std::numeric_limits<float>::infinity(), sign ? -1.0f : 1.0f);
      }
  } else if (std::isinf(Op1)) {
      bool sign = std::signbit(Op1) ^ std::signbit(Op2);
      result = std::copysign(std::numeric_limits<float>::infinity(), sign ? -1.0f : 1.0f);
  } else if (std::isinf(Op2)) {
      bool sign = std::signbit(Op1) ^ std::signbit(Op2);
      result = std::copysign(0.0f, sign ? -1.0f : 1.0f);
  } else {
      result = Op1 / Op2;
  }

  return result;
}

int32_t fcvt_w_s(float f) {
  int32_t result = 0;
  if (std::isnan(f) || f == std::numeric_limits<float>::infinity()) {
      result = std::numeric_limits<int32_t>::max();
  } else if (f == -std::numeric_limits<float>::infinity()) {
      result = std::numeric_limits<int32_t>::min();
  } else {
      long double r = std::nearbyint(static_cast<long double>(f)); // RNE
      if (r > std::numeric_limits<int32_t>::max()) {
          result = std::numeric_limits<int32_t>::max();
      } else if (r < std::numeric_limits<int32_t>::min()) {
          result = std::numeric_limits<int32_t>::min();
      } else {
          result = static_cast<int32_t>(r);
      }
  }

  result = static_cast<int32_t>(result);

  SPDLOG_INFO("converting float {} ({:#x}) -> int32_t {} ({:#x}) ",
    f, std::bit_cast<uint32_t>(f), result, result);

  return result;
}

uint32_t fcvt_wu_s(float f) {
  uint32_t result = 0;
  if (std::isnan(f) || f == std::numeric_limits<float>::infinity()) {
      result = std::numeric_limits<uint32_t>::max();
  } else if (f == -std::numeric_limits<float>::infinity()) {
      result = 0u;
  } else {
      long double r = std::nearbyint(static_cast<long double>(f));
      if (r < 0.0L) {
          result = 0u;
      } else if (r > static_cast<long double>(std::numeric_limits<uint32_t>::max())) {
          result = std::numeric_limits<uint32_t>::max();
      } else {
          result = static_cast<uint32_t>(r);
      }
  }

  SPDLOG_INFO("converting float {} ({:#x}) -> uint32_t {} ({:#x}) ",
    f, std::bit_cast<uint32_t>(f), result, result);
  return result;
}

float fcvt_s_w(int32_t i) {
  float f = static_cast<float>(i);
  SPDLOG_INFO("converting int32 {} -> float {} ({:#x})",
    i, f, std::bit_cast<uint32_t>(f));
  return f;
}

float fcvt_s_wu(uint32_t u) {
  float f = static_cast<float>(u);
  SPDLOG_INFO("converting uint32 {} -> float {} ({:#x})",
    u, f, std::bit_cast<uint32_t>(f));
  return f;
}

} // namespace RVDecoder

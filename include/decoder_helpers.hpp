#include <cstdint>
#include <string>
#include <tuple>
#include <vector>

namespace RVDecoder {
int32_t sign_extend_8_to_32(uint8_t val);
int32_t sign_extend_12_to_32(uint16_t val);
int32_t sign_extend_13_to_32(uint16_t val);
int32_t sign_extend_16_to_32(uint16_t val);
int32_t sign_extend_21_to_32(uint32_t val);
int32_t sign_extend_32_to_32(uint32_t val);
uint32_t zero_extend_16_to_32(uint16_t val);

std::vector<uint32_t> createJalArgs(uint32_t Offset, uint32_t Rd);
std::pair<uint32_t, uint32_t> getJalArgs(std::vector<std::pair<uint32_t, std::string>> Encods);

std::vector<uint32_t> createBArgs(uint32_t Offset, uint32_t Rd);
std::tuple<uint32_t, uint32_t, uint32_t> getBArgs(std::vector<std::pair<uint32_t, std::string>> Encods);

uint32_t getSTOREImm(std::vector<std::pair<uint32_t, std::string>> Encods);

uint32_t classifyS(float Op);

} // namespace RVDecoder

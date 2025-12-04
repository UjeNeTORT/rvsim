// riscv64-unknown-elf-gcc -march=rv32imf_zbb -mabi=ilp32 src/bitwise.c ../api.o -I ../src/ -nostdlib -mno-relax -o bitwise.elf

#include "api.h"
// #include <unistd.h>

const unsigned NInputs = 15;
const unsigned Inputs[15][2] = {
    { 0x00000000u, 0x00000000u }, // both zero
    { 0xFFFFFFFFu, 0xFFFFFFFFu }, // both all ones
    { 0x00000000u, 0xFFFFFFFFu }, // zero vs all ones
    { 0xFFFFFFFFu, 0x00000000u }, // all ones vs zero
    { 0x00000001u, 0x00000001u }, // single bit
    { 0x80000000u, 0x00000001u }, // opposite edge bits
    { 0x7FFFFFFFu, 0x80000000u }, // sign bit vs rest
    { 0x55555555u, 0xAAAAAAAAu }, // alternating patterns
    { 0x12345678u, 0x87654321u }, // random-ish pattern
    { 0x0000FFFFu, 0xFFFF0000u }, // lower vs upper half
    { 0x0F0F0F0Fu, 0xF0F0F0F0u }, // nibble patterns
    { 0x80000000u, 0xFFFFFFFFu }, // sign bit with all ones
    { 0x00000001u, 0xFFFFFFFEu }, // low bit vs all but low
    { 0x7FFFFFFFu, 0x00000001u }, // max positive vs low bit
    { 0xAAAAAAAAu, 0x00000001u }  // alternating with low bit
};

static unsigned Func1(unsigned a, unsigned b) {
    unsigned and_ab = a & b;
    unsigned or_ab  = a | b;
    unsigned xor_ab = a ^ b;

    unsigned andn   = (~a) & b;
    unsigned orn    = a | (~b);
    unsigned xnor   = ~(a ^ b);

    unsigned mix = and_ab ^ or_ab;
    mix ^= xor_ab;
    mix ^= andn;
    mix ^= orn;
    mix ^= xnor;
    mix ^= ~a;
    mix ^= ~b;

    return mix;
}

static int Func2(int a, int b) {
    unsigned sh = b & 31u;

    int sll = a << sh;
    unsigned srl = ((unsigned)a) >> sh;
    int sra = a >> sh;

    int rotl = 0;
    unsigned rotr = 0;

    if (sh == 0) {
        rotl = a;
        rotr = (unsigned)a;
    } else {
        rotl = (int)(((unsigned)a << sh) | ((unsigned)a >> (32u - sh)));
        rotr = ((unsigned)a >> sh) | ((unsigned)a << (32u - sh));
    }

    int r = sll ^ (int)srl;
    r ^= sra;
    r ^= rotl;
    r ^= (int)rotr;

    return r;
}


int main() {

  unsigned Res1[NInputs];
  unsigned Res2[NInputs];

  for (int i = 0; i != NInputs; ++i) {
    Res1[i] = Func1(Inputs[i][0], Inputs[i][1]);
    Res2[i] = Func2(Inputs[i][0], Inputs[i][1]);
  }

  write(1, (const char *) Res1, sizeof(Res1));
  write(1, (const char *) Res2, sizeof(Res2));
  write(1, "\n", 1);
  return 0;
}

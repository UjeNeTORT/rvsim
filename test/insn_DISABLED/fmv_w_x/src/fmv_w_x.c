// riscv64-unknown-elf-gcc -march=rv32imf_zbb -mabi=ilp32 src/fmv_w_x.c ../../api.o -I ../../src/ -nostdlib -mno-relax -o fmv_w_x.elf
#include "api.h"

int main(void) {
  int in = 0;
  float out = 0;

  read(0, (void *)&in, sizeof(int));

  __asm__ volatile(
      "fmv.w.x %0, %1"
      : "=f"(out)
      : "r"(in)
  );

  write(1, (void *)&out, sizeof(float));

  return 0;
}

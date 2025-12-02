// riscv64-unknown-elf-gcc -march=rv32imf_zbb -mabi=ilp32 -Wl,-e,main -nostdlib -mno-relax -static -I ../src/ -o plus.elf src/plus.c ../api.o

#include "api.h"

int main() {
  char a = 1;
  char b = 2;
  char c = a + b + '0';
  write(1 /*stdout*/, &c, 1);
  char n = '\n';
  return write(1, &n, 1);
}

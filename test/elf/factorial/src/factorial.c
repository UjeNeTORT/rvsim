// riscv64-unknown-elf-gcc -march=rv32im -mabi=ilp32 src/factorial.c ../api.o -I ../src/ -nostdlib -mno-relax -o factorial.elf

#include "api.h"

static int fact(int n) {
  if (n < 0) return -1;
  if (n == 0) return 1;
  if (n == 1) return 1;
  int fn_1 = fact(n - 1);
  return fn_1 * n;
}

int main() {
  int number = 0;
  read(0, (char *)&number, 4); // binary
  int res = fact(number);
  write(1, (char *)&res, 4);
  char n = '\n';
  return write(1, &n, 1);
}

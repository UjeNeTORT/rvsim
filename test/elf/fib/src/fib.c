// riscv64-unknown-elf-gcc -march=rv32imf_zbb -mabi=ilp32 src/fib.c ../../api.o -I ../../src/ -nostdlib -mno-relax -o fib.elf

#include "api.h"

static int fib(int n) {
  if (n <= 1) return n;
  return fib(n-1) + fib(n-2);
}

static int fib_iter(int n) {
  if (n <= 1) return n;
  int a = 0, b = 1, c;
  for (int i = 2; i <= n; i++) {
      c = a + b;
      a = b;
      b = c;
  }
  return b;
}

int main() {
  int n = 30;
  int rec_result = fib(n);
  int iter_result = fib_iter(n);

  int ok = 1;
  for (int i = 0; i <= n; i++) {
    if (fib(i) != fib_iter(i)) {
        ok = 0;
    }
  }

  write(1, (void *)&ok, sizeof(int));
  return !ok;
}

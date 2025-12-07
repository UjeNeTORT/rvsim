// riscv64-unknown-elf-gcc -march=rv32imf_zbb -mabi=ilp32 src/argc_argv.c ../api.o -I ../src/ -nostdlib -mno-relax -o argc_argv.elf

#include "api.h"

static int atoi(const char* str) {
  while ( ' ' == *str || '\t' == *str ) {
    ++str;
  }

  int negative = 0;

  if (*str == '+') {
    ++str;
  } else if (*str == '-') {
    ++str;
    negative = 1;
  }

  int result = 0;
  for (; '0' <= *str && *str <= '9'; ++str) {
    int digit = *str - '0';
    result *= 10;
    result -= digit; // calculate in negatives to support INT_MIN, LONG_MIN,..
  }

  return negative ? result : -result;
}

int main(int argc, char* argv[])
{
  if (argc != 2) {
    int* ptr = 0x0;
    *ptr = 666;
  }

  unsigned res = atoi(argv[1]);
  write(1, (char *)&res, sizeof(unsigned));
  return res;
}

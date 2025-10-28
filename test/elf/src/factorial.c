// riscv64-unknown-elf-gcc -march=rv32im -mabi=ilp32 fact.c -nostdlib -mno-relax -S -o fact.s
// riscv64-unknown-elf-gcc -march=rv32im -mabi=ilp32 fact.c -nostdlib -mno-relax -Wl,-e,main -o fact.elf

int fact(int n) {
  if (n < 0) return -1;
  if (n == 0) return 1;
  return fact(n - 1) * n;
}

int main() {
  int f_1 = fact(-1);
  int f0 = fact(0);
  int f1 = fact(1);
  int f2 = fact(2);
  int f5 = fact(5);
  int f10 = fact(10);

  asm volatile (
    "# begin asm inline\n\t"
    "mv x5, %0\n\t" 
    "mv x6, %1\n\t" 
    "mv x7, %2\n\t" 
    "mv x28, %3\n\t" 
    "mv x29, %4\n\t" 
    "mv x30, %5\n\t" 
    "# end asm inline\n\t"
    :
    : "r" (f_1), "r"(f0), "r"(f1), "r"(f2), "r"(f5), "r"(f10)
    : "x5", "x6", "x7", "x28", "x29", "x30"
  );

  return f_1 + f0 + f1 + f2 + f5 + f10;
}


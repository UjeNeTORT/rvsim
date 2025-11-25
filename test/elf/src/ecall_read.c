// riscv64-unknown-elf-gcc -march=rv32imf_zbb -mabi=ilp32 -Wl,-e,main -nostdlib -mno-relax -static -o ../ecall_read.elf ecall_read.c
int main() {
  unsigned fd = 0; // stdin = 0
  char buf[] = "Hello world";
  unsigned count = 6;
  unsigned ret_val = -1;

  asm volatile (
    "add a0, %1, zero\n"
    "add a1, %2, zero\n"
    "add a2, %3, zero\n"
    "li a7, 63\n"
    "ecall\n"
    : "=r" (ret_val)
    : "r"(fd), "r" (buf), "r" (count)
    : "a0", "a1", "a2", "a7"
  );

  return 0;
}

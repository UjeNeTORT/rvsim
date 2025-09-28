int main() {
  unsigned fd = 1; // stdin = 1
  char buf[] = "Hello world";
  unsigned count = 6;
  unsigned ret_val = -1;

  asm volatile (
    "lw a0, (%0)\n"
    "add a1, %1, zero\n"
    "li a7, 63\n"
    "ecall\n"
    : "=r" (ret_val)
    : "r" (buf), "r" (count)
    : "a0", "a1", "a7"
  );

  return 0;
}

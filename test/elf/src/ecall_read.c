int main() {
  unsigned fd = 1; // stdin = 1
  char buf[] = "Hello world";
  unsigned count = 6;
  unsigned ret_val = -1;

  asm volatile (
    "lw a0, (%0)\n"   
    "li a7, 63\n"
    "ecall\n"
    : "=r" (ret_val)
    : "r" (buf) 
    : "a0", "a7"    
  );
  
  return 0;
}


.global _start

.text

_start:
  addi x10, x0, 11 
  addi x14, x0, 1 # shamt = 1
 
  addi x11, x0, 12 
  addi x15, x0, 2 # shamt = 2
  
  addi x12, x0, 100 
  addi x16, x0, 3 # shamt = 3

  sll x10, x10, x14
  sll x11, x11, x15
  sll x12, x12, x16

  ebreak


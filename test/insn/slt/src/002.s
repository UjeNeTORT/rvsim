.global _start

.text

_start:
  addi x10, x0, 1
  addi x11, x0, 2
  slt x1, x10, x11 # 1
  
  addi x12, x0, 1
  addi x13, x0, -1
  slt x2, x12, x13 # 0
  
  addi x14, x0, 0
  addi x15, x0, 1
  slt x3, x14, x15 # 1
  
  addi x16, x0, -1
  addi x17, x0, 1
  slt x4, x16, x17 # 1
  
  addi x18, x0, 1
  addi x19, x0, 1
  slt x5, x18, x19 # 0
  
  addi x20, x0, -1
  addi x21, x0, -111
  slt x6, x20, x21 # 0

  ebreak

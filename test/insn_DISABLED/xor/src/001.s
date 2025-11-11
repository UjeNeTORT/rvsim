.global _start

.text

_start:
  addi x10, x0, 1
  addi x11, x0, 1
  xor x1, x10, x11 # 0 
  
  li x12, 1365
  li x13, 2730
  xor x2, x12, x13 # 11111... 
  
  addi x14, x0, 1
  addi x15, x0, 0
  xor x3, x14, x15 # 1 
  
  addi x16, x0, -1
  addi x17, x0, 15
  xor x4, x16, x17 # easy to calc yourself 

  ebreak


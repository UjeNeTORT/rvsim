.global _start

.text

_start:
  add x0, x11, x12 # behave like nop
  addi x11, x0, 10 # x11 = 10
  addi x12, x0, 16 # x12 = 16
  add x10, x11, x12 # 
  ebreak

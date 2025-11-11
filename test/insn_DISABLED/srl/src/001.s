.global _start

.text

_start:
  li x10, 0x1
  li x11, 0x1
  srl x1, x10, x11 # 0x1
  
  li x12, 0xfff
  li x13, 0x1
  srl x2, x12, x13 # 0x7ff

  li x14, 0xfff
  li x15, 0x4
  srl x3, x14, x15 # 0x0ff

  ebreak


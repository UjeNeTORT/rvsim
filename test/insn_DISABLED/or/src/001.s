.global _start

.text

_start:
  li x10, 0xffffffff
  li x11, 0xffffffff
  or x1, x10, x11
  
  li x12, 0xf0f0f0f0
  li x13, 0x0f0f0f0f
  or x2, x12, x13
  
  li x14, 0xaaaaaaaa
  li x15, 0x55555555
  or x3, x14, x15
  
  li x16, 0x0
  li x17, 0x1
  or x4, x16, x17
  
  li x18, 0x0
  li x19, 0x0
  or x5, x18, x19

  ebreak


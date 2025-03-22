.global _start

.text

_start:
  li x10, 0xffffffff
  li x11, 0xffffffff
  and x1, x10, x11 # 0xfff...
  
  li x12, 0xf0f0f0f0
  li x13, 0x0f0f0f0f
  and x2, x12, x13 # 0x000
  
  li x14, 0xaaaaaaaa
  li x15, 0x55555555
  and x3, x14, x15 # 0x000
  
  li x16, 0x0
  li x17, 0x1
  and x4, x16, x17 # 0x000
  
  li x18, 0x0
  li x19, 0x0
  and x5, x18, x19 # 0x000
  
  li x20, 0x1
  li x21, 0x1
  and x6, x20, x21 # 0x001

  ebreak


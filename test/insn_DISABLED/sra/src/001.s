.global _start

.text

_start:
  li x10, -1
  
  li x11, 0x1  
  li x12, 0x2  
  li x13, 0x3  
  li x14, 0x4  
  li x15, 0x5  
 
  sra x1, x10, x11 # all -1 
  sra x2, x10, x12 
  sra x3, x10, x13 
  sra x4, x10, x14 
  sra x5, x10, x15

  ebreak 

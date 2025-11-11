.global _start

.section .text

_start:  
  add x10, x0, x10
  addi x10, x28, 666
  addi x11, x0, 333

  sub x10, x11, x10 
  ebreak


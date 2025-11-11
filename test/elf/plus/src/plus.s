.global _start

.section .data
number1: .word 6666
number2: .word 1111

.section .text
_start:
  lw a0, number1

  lw a1, number2

  add a0, a0, a1
  
  ebreak

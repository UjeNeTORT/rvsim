.global _start

.section .data
n: .byte 8

.section .rodata
c: .byte 42

.section .text
_start:
  lb a2, n
  jal factorial
  
  add t0, zero, a0

factorial:
  add t3, zero, a2 # t3 = a2
  
  li t1, 1 
  beq t3, t1, fac_end 
  sub t3, t3, t1
  
  add a2, zero, t3
  jal factorial
  mul a0, a0, t3
  
  ret

fac_end:
  li a0, 1
  ret

.global read
.global write
.global exit
.global _start
.section .text

read:
li a7, 63
ecall
ret

write:
li a7, 64
ecall
ret

exit:
li a7, 93
ecall

_start:
lw a0, 0(sp)
addi a1, sp, 4
call main
j exit


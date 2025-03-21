.global _start

.text

_start:
  addi x10, x0, 160
  addi x11, x0, 100
  addi x12, x0, 60
  addi x13, x0, 160

  sub x10, x10, x11 # x10 = 60
  sub x13, x13, x11 # x13 = 60

  bne x11, x12, do_nothing # taken

  ebreak

do_nothing:
  beq x11, x13, do_important # not taken
  ebreak

do_important:
  ebreak

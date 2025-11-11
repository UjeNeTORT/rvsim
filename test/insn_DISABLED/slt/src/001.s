.global _start

.text

_start:
  addi x10, x0, 666
  addi x21, x0, 665

  slt x1, x10, x21

  ebreak

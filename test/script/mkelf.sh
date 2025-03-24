#!/usr/bin/bash

# usage
# ./mkelf.sh <asm file> <output destination>

set -e

riscv32-elf-as $1 -o $1.o -fno-pic
riscv32-elf-ld $1.o -o $2 -static --no-relax

rm -fv $1.o 1>&2

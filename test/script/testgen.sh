#!/bin/bash

# usage:
# ./testgen.sh ../insn/add/src/001.s > ../insn/add/001.bstate

echoerr() { echo "$@" 1>&2; }

#set -x # verbose
set -e # exit on error

INPUT_ASM=$1
N_ARGS=$#

INPUT_REG="regs_zero.bin"

MODEL_SIGN="MODEL_SIGN.bin"
MEM_SIGN="MEM_SIGN.bin"

if [ $N_ARGS -ge 2 ]; then
  INPUT_REG=$2
fi

echoerr "# Generating input binary state for \"$INPUT_ASM\""
echoerr "# asm  = \"$INPUT_ASM\""
echoerr "# regs = \"$INPUT_REG\""
echoerr
echoerr "# model sign = \"$MODEL_SIGN\""
echoerr "# memory sign = \"$MEM_SIGN\""

OUTPUT_ELF=$INPUT_ASM.elf

riscv32-elf-as $INPUT_ASM -o $INPUT_ASM.o
wait
START_PC=$(objdump $INPUT_ASM.o -h | grep .text | awk '{print $6}')
# riscv32-elf-ld $INPUT_ASM.o -o $OUTPUT_ELF

wait 

# cat $MODEL_SIGN <(printf "%08x" "0x$START_PC" | tac -rs .. | tr -d '\n' | xxd -r -p) $INPUT_REG $MEM_SIGN $INPUT_ASM.o
./fromobj.sh $MODEL_SIGN $INPUT_REG $MEM_SIGN $INPUT_ASM.o 

wait

rm -v $INPUT_ASM.o 1>&2


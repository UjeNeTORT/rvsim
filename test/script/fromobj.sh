#!/usr/bin/bash

# usage
# ./fromobj.sh $MODEL_SIGN $INPUT_REG $MEM_SIGN $INPUT_ASM.o > somewhere 
#               const      registers   const     OBJECT  

START_PC=$(riscv32-elf-objdump $4 -h | grep .text | awk '{print $6}')
echo "# start pc = $START_PC" 1>&2

wait 
cat $1 <(printf "%08x" "0x$START_PC" | tac -rs .. | tr -d '\n' | xxd -r -p) $2 $3 $4


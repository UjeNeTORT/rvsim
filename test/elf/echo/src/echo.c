// riscv64-unknown-elf-gcc -march=rv32im -mabi=ilp32 src/factorial.c ../api.o -I ../src/ -nostdlib -mno-relax -o factorial.elf

#include "api.h"

int main( int argc, char* argv[])
{
char data[1024];

return write(1, data, read( 0, data, sizeof(data)));
}

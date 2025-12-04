// riscv64-unknown-elf-gcc -march=rv32imf_zbb -mabi=ilp32 src/echo.c ../api.o -I ../src/ -nostdlib -mno-relax -o echo.elf

#include "api.h"

int main( int argc, char* argv[])
{
char data[1024];

return write(1, data, read( 0, data, sizeof(data)));
}

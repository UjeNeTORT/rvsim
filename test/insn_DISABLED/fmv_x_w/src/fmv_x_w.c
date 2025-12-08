// riscv64-unknown-elf-gcc -march=rv32imf_zbb -mabi=ilp32 src/fmv_x_w.c ../../api.o -I ../../src/ -nostdlib -mno-relax -o fmv_x_w.elf
#include "api.h"

int main(void) {
    float in;
    int out;

    read(0, (void *)&in, sizeof(float));

    __asm__ volatile(
        "fmv.x.w %0, %1"
        : "=r"(out)
        : "f"(in)
    );

    write(1, (void *)&out, 4);

    return 0;
}

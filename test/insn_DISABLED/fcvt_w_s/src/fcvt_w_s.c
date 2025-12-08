// riscv64-unknown-elf-gcc -march=rv32imf_zbb -mabi=ilp32 src/fcvt_w_s.c ../../api.o -I ../../src/ -nostdlib -mno-relax -o fcvt_w_s.elf
#include "api.h"

int main(void) {
    float in = 0.0f;
    int out = 0;

    read(0, (void *)&in, sizeof(float));
    __asm__ volatile(
        "fcvt.w.s %0, %1"
        : "=r"(out)
        : "f"(in)
    );

    write(1, (void *)&out, sizeof(int));
    return 0;
}

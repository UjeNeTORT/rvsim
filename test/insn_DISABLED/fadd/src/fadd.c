// riscv64-unknown-elf-gcc -march=rv32imf_zbb -mabi=ilp32 src/fadd.c ../../api.o -I ../../src/ -nostdlib -mno-relax -o fadd.elf
#include "api.h"

static float fadd(float a, float b) {
    float r;
    __asm__ volatile (
        "fadd.s %0, %1, %2"
        : "=f"(r)
        : "f"(a), "f"(b)
    );
    return r;
}

int main(void) {
    float inbuf[2];
    float out;

    read(0, (void *)inbuf, sizeof(inbuf));
    out = fadd(inbuf[0], inbuf[1]);
    write(1, (void *)&out, sizeof(out));

    return 0;
}

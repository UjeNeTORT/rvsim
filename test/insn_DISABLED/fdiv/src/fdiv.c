// riscv64-unknown-elf-gcc -march=rv32imf_zbb -mabi=ilp32 src/fdiv.c ../../api.o -I ../../src/ -nostdlib -mno-relax -o fdiv.elf
#include "api.h"

static float fdiv(float a, float b) {
    float r;
    __asm__ volatile(
        "fdiv.s %0, %1, %2"
        : "=f"(r)
        : "f"(a), "f"(b)
    );
    return r;
}

int main(void) {
    float inbuf[2];
    float out;

    read(0, (void *)inbuf, sizeof(inbuf));

    out = fdiv(inbuf[0], inbuf[1]);

    write(1, (void *)&out, sizeof(out));

    return 0;
}

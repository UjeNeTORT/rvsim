// riscv64-unknown-elf-gcc -march=rv32imf_zbb -mabi=ilp32 src/div.c ../../api.o -I ../../src/ -nostdlib -mno-relax -o div.elf
#include "api.h"

static unsigned rv32m_divu(unsigned a, unsigned b)
{
    unsigned res;
    __asm__ volatile ("divu %0, %1, %2" : "=r"(res) : "r"(a), "r"(b));
    return res;
}

static unsigned rv32m_remu(unsigned a, unsigned b)
{
    unsigned res;
    __asm__ volatile ("remu %0, %1, %2" : "=r"(res) : "r"(a), "r"(b));
    return res;
}

struct Input {
    unsigned dividend;
    unsigned divisor;
};

struct Output {
    unsigned quotient;
    unsigned remainder;
};

int main(void)
{
    struct Input in;

    if (!read_exact(&in, (long)sizeof(in))) {
        return 1;
    }

    struct Output out;
    out.quotient  = rv32m_divu(in.dividend, in.divisor);
    out.remainder = rv32m_remu(in.dividend, in.divisor);

    write_all(&out, (long)sizeof(out));
    return 0;
}

// riscv64-unknown-elf-gcc -march=rv32imf_zbb -mabi=ilp32 src/mul.c ../../../api.o -I ../../../src/ -nostdlib -mno-relax -o mul.elf
#include "api.h"

static unsigned rv32m_mul(unsigned a, unsigned b)
{
    unsigned res;
    __asm__ volatile ("mul %0, %1, %2" : "=r"(res) : "r"(a), "r"(b));
    return res;
}

static unsigned rv32m_mulh(unsigned a, unsigned b)
{
    unsigned res;
    __asm__ volatile ("mulh %0, %1, %2" : "=r"(res) : "r"(a), "r"(b));
    return res;
}

static unsigned rv32m_mulhsu(unsigned a, unsigned b)
{
    unsigned res;
    __asm__ volatile ("mulhsu %0, %1, %2" : "=r"(res) : "r"(a), "r"(b));
    return res;
}

static unsigned rv32m_mulhu(unsigned a, unsigned b)
{
    unsigned res;
    __asm__ volatile ("mulhu %0, %1, %2" : "=r"(res) : "r"(a), "r"(b));
    return res;
}

struct Input {
    unsigned a;
    unsigned b;
};

struct Output {
    unsigned mul;
    unsigned mulh;
    unsigned mulhu;
    unsigned mulhsu;
};

int main(void)
{
    struct Input in;

    if (!read_exact(&in, (long)sizeof(in))) {
        return 1;
    }

    struct Output out;
    out.mul   = rv32m_mul(in.a, in.b);
    out.mulh  = rv32m_mulh(in.a, in.b);
    out.mulhu = rv32m_mulhu(in.a, in.b);
    out.mulhsu= rv32m_mulhsu(in.a, in.b);

    write_all(&out, (long)sizeof(out));
    return 0;
}

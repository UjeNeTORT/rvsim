// riscv64-unknown-elf-gcc -march=rv32imf_zbb -mabi=ilp32 -g src/ld_st.c ../../api.o -I ../../src/ -nostdlib -mno-relax -o ld_st.elf
#include "api.h"

struct Input {
    int  s8_base;
    unsigned u8_base;
    int  s16_base;
    unsigned u16_base;
    int  s32_base;
    unsigned u32_base;
};

struct Output {
    int sum;
};

static char            s8arr[4];
static unsigned char   u8arr[4];
static short          s16arr[4];
static unsigned short u16arr[4];
static int            s32arr[4];
static unsigned       u32arr[4];

int main(void)
{
    struct Input in;

    if (!read_exact(&in, (long)sizeof(in))) {
        return 1;
    }

    for (int i = 0; i < 4; i++) {
        s8arr[i]   = (char)(in.s8_base - i);
        u8arr[i]   = (unsigned char)(in.u8_base + (unsigned)i);
        s16arr[i]  = (short)(in.s16_base - i);
        u16arr[i]  = (unsigned short)(in.u16_base + (unsigned)i);
        s32arr[i]  = in.s32_base - i;
        u32arr[i]  = in.u32_base + (unsigned)i;
    }

    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += s8arr[i];
        sum += (int)u8arr[i];
        sum += s16arr[i];
        sum += (int)u16arr[i];
        sum += s32arr[i];
        sum += (int)u32arr[i];
    }

    struct Output out = { sum };
    write_all(&out, (long)sizeof(out));
    return 0;
}

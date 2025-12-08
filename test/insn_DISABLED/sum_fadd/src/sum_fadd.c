// riscv64-unknown-elf-gcc -march=rv32imf_zbb -mabi=ilp32 src/sum_fadd.c ../../api.o -I ../../src/ -nostdlib -mno-relax -o sum_fadd.elf
#include "api.h"

#define CREATE_TEST( name, type0, operation, type1, type_res)   \
    int name( type0 op0, type1 op1, type_res res)               \
    {                                               \
        if ( ((op0) operation (op1)) != (res) )     \
        {                                           \
            return 1;                               \
        }                                           \
        return 0;                                   \
    }


CREATE_TEST(add_f32f32, float, +, float, float)
CREATE_TEST(add_f32s32, float, +, int, float)
CREATE_TEST(add_s32f32, int, +, float, float)

int main(int argc, char* argv[])
{
  int res =
    add_f32f32( 3.11f, 2.7f, 5.81f)   +
    add_f32f32( 1.5f, 2.5f, 4.0f)   +
    add_f32f32( 0.0f, 0.0f, 0.0f)   +

    add_f32s32( 3.14f, 10, 13.14f) +
    add_f32s32( -5.25f, 10, 4.75f)   +
    add_f32s32( 7.0f, -3, 4.0f)   +

    add_s32f32( 10, 3.14f, 13.14f)   +
    add_s32f32( -10, -5.25f, -15.25f)   +
    add_s32f32( 20, 7.0f, 27.0f);

  write(1, (void *)&res, sizeof(int));
  return res;
}

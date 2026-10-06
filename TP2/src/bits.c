#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

int main(void)
{
    /* Bits 4 et 20 en partant de la gauche d'un mot de 32 bits. */
    uint32_t d = UINT32_C(0x10001000);
    uint32_t bit_4 = (d >> 28) & UINT32_C(1);
    uint32_t bit_20 = (d >> 12) & UINT32_C(1);

    printf("%" PRIu32 "\n", bit_4 == 1 && bit_20 == 1);
    return 0;
}
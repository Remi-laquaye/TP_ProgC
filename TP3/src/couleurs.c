#include <stdio.h>

typedef struct {
    unsigned char rouge;
    unsigned char vert;
    unsigned char bleu;
    unsigned char alpha;
} Couleur;

int main(void)
{
    const Couleur couleurs[] = {
        {0xef, 0x78, 0x12, 0xff},
        {0x2c, 0xc8, 0x64, 0xff},
        {0x33, 0x66, 0x99, 0xff},
        {0xff, 0x00, 0x00, 0xff},
        {0x00, 0xff, 0x00, 0xff},
        {0x00, 0x00, 0xff, 0xff},
        {0xff, 0xff, 0x00, 0xff},
        {0xff, 0x00, 0xff, 0xff},
        {0x00, 0xff, 0xff, 0xff},
        {0x80, 0x80, 0x80, 0xff}
    };
    const size_t nombre = sizeof couleurs / sizeof couleurs[0];

    for (size_t i = 0; i < nombre; ++i) {
        printf("Couleur %zu : R=%u G=%u B=%u A=%u\n", i + 1,
               (unsigned int)couleurs[i].rouge,
               (unsigned int)couleurs[i].vert,
               (unsigned int)couleurs[i].bleu,
               (unsigned int)couleurs[i].alpha);
    }
    return 0;
}
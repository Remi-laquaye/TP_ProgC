#include <stdio.h>

typedef struct {
    unsigned char rouge;
    unsigned char vert;
    unsigned char bleu;
    unsigned char alpha;
} Couleur;

int main(void)
{
    const Couleur couleurs[10] = {
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

    for (int i = 0; i < 10; ++i) {
        printf("Couleur %d :\n", i + 1);
        printf("Rouge : %u\n", (unsigned int)couleurs[i].rouge);
        printf("Vert : %u\n", (unsigned int)couleurs[i].vert);
        printf("Bleu : %u\n", (unsigned int)couleurs[i].bleu);
        printf("Alpha : %u\n\n", (unsigned int)couleurs[i].alpha);
    }
    return 0;
}
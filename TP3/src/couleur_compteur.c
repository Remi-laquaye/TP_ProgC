#include <stdio.h>

#define TAILLE 100

typedef struct {
    unsigned char rouge;
    unsigned char vert;
    unsigned char bleu;
    unsigned char alpha;
} Couleur;

typedef struct {
    Couleur couleur;
    int occurrences;
} CompteurCouleur;

static int memes_couleurs(Couleur gauche, Couleur droite)
{
    return gauche.rouge == droite.rouge &&
           gauche.vert == droite.vert &&
           gauche.bleu == droite.bleu &&
           gauche.alpha == droite.alpha;
}

int main(void)
{
    const Couleur palette[5] = {
        {0xff, 0x23, 0x23, 0x45},
        {0xff, 0x00, 0x23, 0x12},
        {0x10, 0x20, 0x30, 0xff},
        {0x00, 0x80, 0xff, 0xff},
        {0x80, 0x40, 0x20, 0xff}
    };
    Couleur couleurs[TAILLE];
    CompteurCouleur distinctes[TAILLE];
    int nombre_distinctes = 0;

    for (int i = 0; i < TAILLE; ++i) {
        couleurs[i] = palette[(i * 7 + i / 5) % 5];
    }

    for (int i = 0; i < TAILLE; ++i) {
        int j;
        for (j = 0; j < nombre_distinctes; ++j) {
            if (memes_couleurs(couleurs[i], distinctes[j].couleur)) {
                ++distinctes[j].occurrences;
                break;
            }
        }
        if (j == nombre_distinctes) {
            distinctes[nombre_distinctes].couleur = couleurs[i];
            distinctes[nombre_distinctes].occurrences = 1;
            ++nombre_distinctes;
        }
    }

    for (int i = 0; i < nombre_distinctes; ++i) {
        Couleur couleur = distinctes[i].couleur;
        printf("0x%02x 0x%02x 0x%02x 0x%02x : %d\n",
               (unsigned int)couleur.rouge, (unsigned int)couleur.vert,
               (unsigned int)couleur.bleu, (unsigned int)couleur.alpha,
               distinctes[i].occurrences);
    }
    return 0;
}
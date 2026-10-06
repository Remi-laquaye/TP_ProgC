#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TAILLE 11

static void afficher_entiers(const int *tableau)
{
    const int *p = tableau;
    const int *fin = tableau + TAILLE;

    while (p < fin) {
        printf("%s%d", p == tableau ? "" : ", ", *p);
        ++p;
    }
    printf("\n");
}

static void afficher_reels(const float *tableau)
{
    const float *p = tableau;
    const float *fin = tableau + TAILLE;

    while (p < fin) {
        printf("%s%.2f", p == tableau ? "" : ", ", (double)*p);
        ++p;
    }
    printf("\n");
}

int main(void)
{
    int entiers[TAILLE];
    float reels[TAILLE];
    int *p_entiers = entiers;
    float *p_reels = reels;

    srand((unsigned int)time(NULL));
    for (int i = 0; i < TAILLE; ++i) {
        *(p_entiers + i) = rand() % 100;
        *(p_reels + i) = (float)(rand() % 1000) / 100.0f;
    }

    printf("Tableau d'entiers (avant la multiplication par 3) :\n");
    afficher_entiers(entiers);
    printf("Tableau de reels (avant la multiplication par 3) :\n");
    afficher_reels(reels);

    for (int i = 0; i < TAILLE; ++i) {
        if (i % 2 == 0) {
            *(p_entiers + i) *= 3;
            *(p_reels + i) *= 3.0f;
        }
    }

    printf("Tableau d'entiers (apres la multiplication par 3) :\n");
    afficher_entiers(entiers);
    printf("Tableau de reels (apres la multiplication par 3) :\n");
    afficher_reels(reels);
    return 0;
}
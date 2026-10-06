#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TAILLE 100

static void afficher_tableau(const int tableau[TAILLE])
{
    for (int i = 0; i < TAILLE; ++i) {
        printf("%d%c", tableau[i], i == TAILLE - 1 ? '\n' : ' ');
    }
}

int main(void)
{
    int tableau[TAILLE];

    srand((unsigned int)time(NULL));
    for (int i = 0; i < TAILLE; ++i) {
        tableau[i] = rand() % 201 - 100;
    }

    printf("Tableau non trie :\n");
    afficher_tableau(tableau);

    for (int i = 1; i < TAILLE; ++i) {
        int valeur = tableau[i];
        int j = i;
        while (j > 0 && tableau[j - 1] > valeur) {
            tableau[j] = tableau[j - 1];
            --j;
        }
        tableau[j] = valeur;
    }

    printf("Tableau trie par ordre croissant :\n");
    afficher_tableau(tableau);
    return 0;
}
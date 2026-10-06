#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TAILLE 100

int main(void)
{
    int tableau[TAILLE];
    int recherche;
    int present = 0;

    srand((unsigned int)time(NULL));
    for (int i = 0; i < TAILLE; ++i) {
        tableau[i] = rand() % 201 - 100;
    }

    printf("Tableau :\n");
    for (int i = 0; i < TAILLE; ++i) {
        printf("%d%c", tableau[i], i == TAILLE - 1 ? '\n' : ' ');
    }

    printf("Entrez l'entier que vous souhaitez chercher : ");
    if (scanf("%d", &recherche) != 1) {
        fprintf(stderr, "Entree invalide.\n");
        return 1;
    }

    for (int i = 0; i < TAILLE; ++i) {
        if (tableau[i] == recherche) {
            present = 1;
            break;
        }
    }

    printf("Resultat : entier %s\n", present ? "present" : "absent");
    return 0;
}
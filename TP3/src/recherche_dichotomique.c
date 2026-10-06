#include <stdio.h>

#define TAILLE 100

int main(void)
{
    int tableau[TAILLE];
    int recherche;
    int debut = 0;
    int fin = TAILLE - 1;
    int present = 0;

    for (int i = 0; i < TAILLE; ++i) {
        tableau[i] = i * 3 - 100;
    }

    printf("Tableau trie :\n");
    for (int i = 0; i < TAILLE; ++i) {
        printf("%d%c", tableau[i], i == TAILLE - 1 ? '\n' : ' ');
    }

    printf("Entrez l'entier que vous souhaitez chercher : ");
    if (scanf("%d", &recherche) != 1) {
        fprintf(stderr, "Entree invalide.\n");
        return 1;
    }

    while (debut <= fin) {
        int milieu = debut + (fin - debut) / 2;
        if (tableau[milieu] == recherche) {
            present = 1;
            break;
        }
        if (tableau[milieu] < recherche) {
            debut = milieu + 1;
        } else {
            fin = milieu - 1;
        }
    }

    printf("Resultat : entier %s\n", present ? "present" : "absent");
    return 0;
}
#include <stdio.h>

int main(void) {
    int compteur = 5; // Valeur strictement inférieure à 10

    if (compteur >= 10 || compteur <= 0) {
        printf("Le compteur doit etre strictement inferieur a 10 et positif.\n");
        return 1;
    }

    /* =======================================================
       VERSION 1 : BOUCLES FOR
       ======================================================= */
    printf("=== VERSION BOUCLES FOR (compteur = %d) ===\n", compteur);

    for (int i = 0; i < compteur; i++) {
        for (int j = 0; j <= i; j++) {
            /* Condition pour afficher un astérisque '*' :
               - Première colonne (j == 0)
               - Dernière colonne de la ligne actuelle (j == i)
               - Dernière ligne complète (i == compteur - 1) */
            if (j == 0 || j == i || i == compteur - 1) {
                printf("* ");
            } else {
                printf("# ");
            }
        }
        printf("\n");
    }

    printf("\n");

    /* =======================================================
       VERSION 2 : BOUCLE WHILE (avec break et continue)
       ======================================================= */
    printf("=== VERSION BOUCLE WHILE (compteur = %d) ===\n", compteur);

    int i = 0;
    while (1) { // Boucle infinie contrôlée par 'break'
        if (i >= compteur) {
            break; // Condition de sortie
        }

        int j = 0;
        while (1) {
            if (j > i) {
                break; // Fin de la ligne actuelle
            }

            // Gestion des contours
            if (j == 0 || j == i || i == compteur - 1) {
                printf("* ");
                j++;
                continue; // Passe directement à l'itération suivante de la boucle interne
            }

            // Gestion de l'intérieur
            printf("# ");
            j++;
        }

        printf("\n");
        i++;
    }

    return 0;
}
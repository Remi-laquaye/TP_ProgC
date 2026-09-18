#include <stdio.h>

int main(void) {
    /* 1. Déclaration et 2. Initialisation des variables */
    int n = 5; // Hauteur de la pyramide
    int i, j;

    printf("=== GÉNÉRATION DE LA PYRAMIDE (hauteur = %d) ===\n\n", n);

    /* 3. Boucle de génération de la pyramide */
    for (i = 1; i <= n; i++) {

        /* a. Boucle pour afficher les espaces de centrage */
        for (j = 1; j <= n - i; j++) {
            printf(" ");
        }

        /* b. Boucle pour afficher les nombres croissants (1 jusqu'à i) */
        for (j = 1; j <= i; j++) {
            printf("%d", j);
        }

        /* c. Boucle pour afficher les nombres décroissants (i-1 jusqu'à 1) */
        for (j = i - 1; j >= 1; j--) {
            printf("%d", j);
        }

        /* 4. Passage à la ligne suivante */
        printf("\n");
    }

    /*5. Terminaison */
    printf("\nGénération de la pyramide terminée avec succès !\n");

    return 0;
}
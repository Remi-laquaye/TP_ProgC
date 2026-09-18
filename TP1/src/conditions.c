#include <stdio.h>

int main(void) {
    int somme = 0;
    int dernier_nombre = 0;

    for (int i = 1; i <= 1000; i++) {
        /* 1. Si le nombre est divisible par 11, on l'ignore et on passe au suivant */
        if (i % 11 == 0) {
            continue;
        }

        /* 2. Si le nombre est divisible par 5 OU par 7, on l'ajoute à la somme */
        if (i % 5 == 0 || i % 7 == 0) {
            somme += i;
            dernier_nombre = i;
        }

        /* 3. Si la somme dépasse 5000, on arrête immédiatement la boucle */
        if (somme > 5000) {
            break;
        }
    }

    /* 4. Affichage du résultat final */
    printf("=== RÉSULTAT DU CALCUL ===\n");
    printf("Somme finale : %d\n", somme);
    printf("Dernier nombre traite : %d\n", dernier_nombre);

    return 0;
}
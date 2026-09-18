#include <stdio.h>

void afficher_binaire(int n) {
    printf("Nombre : %-6d -> Binaire : ", n);

    /* Cas particulier pour 0 */
    if (n == 0) {
        printf("0\n");
        return;
    }

    /* -----------------------------------------------------------------
       MÉTHODE 1 : Décalage de bits (recommandée avec &, >> et boucle for)
       On parcourt les 32 bits de l'entier du plus fort (31) au plus faible (0).
    ----------------------------------------------------------------- */
    int bit_commence = 0; // Pour éviter d'afficher les 0 inutiles au début

    for (int i = (sizeof(int) * 8) - 1; i >= 0; i--) {
        // Décalage de 'i' positions vers la droite puis masque avec 1
        int bit = (n >> i) & 1;

        if (bit == 1) {
            bit_commence = 1; // On trouve le premier '1' significatif
        }

        if (bit_commence) {
            printf("%d", bit);
        }
    }
    printf("\n");
}

/* -----------------------------------------------------------------
   MÉTHODE 2 : Divisions et restes par 2 (% et /)
   Stocke les restes dans un tableau puis affiche à l'envers.
   ----------------------------------------------------------------- */
void afficher_binaire_division(int n) {
    if (n == 0) {
        printf("0\n");
        return;
    }

    int restes[32];
    int taille = 0;
    int temp = n;

    while (temp > 0) {
        restes[taille] = temp % 2;
        temp = temp / 2;
        taille++;
    }

    // Affichage inversé
    for (int i = taille - 1; i >= 0; i--) {
        printf("%d", restes[i]);
    }
    printf("\n");
}

int main(void) {
    int nombres[] = {0, 4096, 65536, 65535, 1024};
    int nb_elements = sizeof(nombres) / sizeof(nombres[0]);

    printf("=== AFFICHAGE BINAIRE (MÉTHODE BIT À BIT) ===\n\n");

    for (int i = 0; i < nb_elements; i++) {
        afficher_binaire(nombres[i]);
    }

    return 0;
}
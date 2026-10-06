#include "fichier.h"

#include <stdio.h>

int lire_fichier(const char *nom_de_fichier)
{
    FILE *fichier = fopen(nom_de_fichier, "r");
    int caractere;

    if (fichier == NULL) {
        perror(nom_de_fichier);
        return 0;
    }

    printf("Contenu du fichier %s :\n", nom_de_fichier);
    while ((caractere = fgetc(fichier)) != EOF) {
        if (putchar(caractere) == EOF) {
            fprintf(stderr, "Erreur lors de l'affichage du fichier.\n");
            fclose(fichier);
            return 0;
        }
    }
    if (ferror(fichier)) {
        perror("Erreur de lecture");
        fclose(fichier);
        return 0;
    }
    if (fclose(fichier) == EOF) {
        perror("Erreur de fermeture");
        return 0;
    }
    return 1;
}

int ecrire_dans_fichier(const char *nom_de_fichier, const char *message)
{
    FILE *fichier = fopen(nom_de_fichier, "w");

    if (fichier == NULL) {
        perror(nom_de_fichier);
        return 0;
    }
    if (fputs(message, fichier) == EOF) {
        perror("Erreur d'ecriture");
        fclose(fichier);
        return 0;
    }
    if (fclose(fichier) == EOF) {
        perror("Erreur de fermeture");
        return 0;
    }
    printf("Le message a ete ecrit dans le fichier %s.\n", nom_de_fichier);
    return 1;
}
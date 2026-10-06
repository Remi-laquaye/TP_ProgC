
#include "fichier.h"
#include "liste.h"
#include "operator.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAILLE_ENTREE 512

static int lire_ligne(const char *invite, char *destination, size_t capacite)
{
    size_t longueur;

    printf("%s", invite);
    if (fgets(destination, (int)capacite, stdin) == NULL) {
        return 0;
    }
    longueur = strlen(destination);
    if (longueur > 0 && destination[longueur - 1] == '\n') {
        destination[longueur - 1] = '\0';
        return 1;
    }
    if (!feof(stdin)) {
        int caractere;
        while ((caractere = getchar()) != '\n' && caractere != EOF) {
        }
        fprintf(stderr, "Saisie trop longue.\n");
        return 0;
    }
    return 1;
}

static int lire_entier(const char *invite, int *valeur)
{
    char ligne[TAILLE_ENTREE];
    char *fin;
    long resultat;

    if (!lire_ligne(invite, ligne, sizeof ligne)) {
        return 0;
    }
    errno = 0;
    resultat = strtol(ligne, &fin, 10);
    if (ligne == fin || *fin != '\0' || errno == ERANGE ||
        resultat < INT_MIN || resultat > INT_MAX) {
        fprintf(stderr, "Veuillez saisir un entier valide.\n");
        return 0;
    }
    *valeur = (int)resultat;
    return 1;
}

static int exercice_operateurs(void)
{
    char ligne[TAILLE_ENTREE];
    int num1;
    int num2;
    int resultat;

    if (!lire_entier("Entrez num1 : ", &num1) ||
        !lire_entier("Entrez num2 : ", &num2) ||
        !lire_ligne("Entrez l'operateur (+, -, *, /, %, &, |, ~) : ",
                    ligne, sizeof ligne)) {
        return 0;
    }
    if (ligne[0] == '\0' || ligne[1] != '\0' ||
        !operation(ligne[0], num1, num2, &resultat)) {
        fprintf(stderr, "Operateur invalide ou operation impossible.\n");
        return 0;
    }
    printf("Resultat : %d\n", resultat);
    return 1;
}

static int exercice_fichiers(void)
{
    char choix[TAILLE_ENTREE];
    char nom_de_fichier[TAILLE_ENTREE];
    char message[TAILLE_ENTREE];

    printf("Que souhaitez-vous faire ?\n");
    printf("1. Lire un fichier\n2. Ecrire dans un fichier\n");
    if (!lire_ligne("Votre choix : ", choix, sizeof choix) ||
        !lire_ligne("Entrez le nom du fichier : ", nom_de_fichier,
                    sizeof nom_de_fichier) || nom_de_fichier[0] == '\0') {
        fprintf(stderr, "Saisie invalide.\n");
        return 0;
    }

    if (strcmp(choix, "1") == 0) {
        return lire_fichier(nom_de_fichier);
    }
    if (strcmp(choix, "2") == 0) {
        if (!lire_ligne("Entrez le message a ecrire : ", message,
                        sizeof message)) {
            return 0;
        }
        return ecrire_dans_fichier(nom_de_fichier, message);
    }
    fprintf(stderr, "Choix invalide.\n");
    return 0;
}

static int exercice_liste(void)
{
    const struct couleur couleurs[10] = {
        {0xff, 0x00, 0x00, 0xff}, {0x00, 0xff, 0x00, 0xff},
        {0x00, 0x00, 0xff, 0xff}, {0xff, 0xff, 0x00, 0xff},
        {0xff, 0x00, 0xff, 0xff}, {0x00, 0xff, 0xff, 0xff},
        {0x80, 0x40, 0x20, 0xff}, {0x12, 0x34, 0x56, 0xff},
        {0x78, 0x9a, 0xbc, 0xff}, {0xef, 0x78, 0x12, 0xff}
    };
    struct liste_couleurs liste;

    init_liste(&liste);
    for (size_t i = 0; i < sizeof couleurs / sizeof couleurs[0]; ++i) {
        if (!insertion(&couleurs[i], &liste)) {
            detruire_liste(&liste);
            return 0;
        }
    }
    printf("Liste des couleurs :\n");
    parcours(&liste);
    detruire_liste(&liste);
    return 1;
}

int main(void)
{
    int choix;

    printf("Choisissez un exercice :\n");
    printf("1. Operateurs (4.1)\n2. Gestion de fichiers (4.2)\n");
    printf("7. Liste de couleurs (4.7)\n");
    if (!lire_entier("Votre choix : ", &choix)) {
        return 1;
    }

    switch (choix) {
    case 1:
        return exercice_operateurs() ? 0 : 1;
    case 2:
        return exercice_fichiers() ? 0 : 1;
    case 7:
        return exercice_liste() ? 0 : 1;
    default:
        fprintf(stderr, "Choix d'exercice invalide.\n");
        return 1;
    }
}

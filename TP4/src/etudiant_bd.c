#include "fichier.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NOMBRE_ETUDIANTS 5
#define TAILLE_NOM 64
#define TAILLE_ADRESSE 256
#define TAILLE_FICHIER 4096

typedef struct {
    char nom[TAILLE_NOM];
    char prenom[TAILLE_NOM];
    char adresse[TAILLE_ADRESSE];
    float note1;
    float note2;
} Etudiant;

static int lire_texte(const char *invite, char *destination, size_t capacite)
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

static int lire_note(const char *invite, float *note)
{
    char ligne[128];
    char *fin;

    if (!lire_texte(invite, ligne, sizeof ligne)) {
        return 0;
    }
    *note = strtof(ligne, &fin);
    if (ligne == fin || *fin != '\0') {
        fprintf(stderr, "Veuillez saisir une note valide.\n");
        return 0;
    }
    return 1;
}

int main(void)
{
    Etudiant etudiants[NOMBRE_ETUDIANTS];
    char contenu[TAILLE_FICHIER];
    size_t utilise = 0;

    for (int i = 0; i < NOMBRE_ETUDIANTS; ++i) {
        printf("Entrez les details de l'etudiant.e %d :\n", i + 1);
        if (!lire_texte("Nom : ", etudiants[i].nom,
                        sizeof etudiants[i].nom) ||
            !lire_texte("Prenom : ", etudiants[i].prenom,
                        sizeof etudiants[i].prenom) ||
            !lire_texte("Adresse : ", etudiants[i].adresse,
                        sizeof etudiants[i].adresse)) {
            return 1;
        }
        if (!lire_note("Note 1 : ", &etudiants[i].note1)) {
            return 1;
        }
        if (!lire_note("Note 2 : ", &etudiants[i].note2)) {
            return 1;
        }
    }

    for (int i = 0; i < NOMBRE_ETUDIANTS; ++i) {
        int ecrit = snprintf(contenu + utilise, sizeof contenu - utilise,
                             "%s;%s;%s;%.2f;%.2f\n",
                             etudiants[i].nom, etudiants[i].prenom,
                             etudiants[i].adresse, (double)etudiants[i].note1,
                             (double)etudiants[i].note2);
        if (ecrit < 0 || (size_t)ecrit >= sizeof contenu - utilise) {
            fprintf(stderr, "Les donnees depassent la capacite du fichier.\n");
            return 1;
        }
        utilise += (size_t)ecrit;
    }

    if (!ecrire_dans_fichier("etudiant.txt", contenu)) {
        return 1;
    }
    printf("Les details des etudiants ont ete enregistres dans "
           "le fichier etudiant.txt.\n");
    return 0;
}
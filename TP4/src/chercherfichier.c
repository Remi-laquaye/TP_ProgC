#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *lire_ligne(FILE *fichier, int *erreur)
{
    size_t capacite = 128;
    size_t longueur = 0;
    char *ligne = malloc(capacite);
    int caractere;

    *erreur = 0;
    if (ligne == NULL) {
        *erreur = 1;
        return NULL;
    }
    while ((caractere = fgetc(fichier)) != EOF && caractere != '\n') {
        if (longueur + 1 >= capacite) {
            size_t nouvelle_capacite = capacite * 2;
            char *nouvelle_ligne = realloc(ligne, nouvelle_capacite);
            if (nouvelle_ligne == NULL) {
                free(ligne);
                *erreur = 1;
                return NULL;
            }
            ligne = nouvelle_ligne;
            capacite = nouvelle_capacite;
        }
        ligne[longueur++] = (char)caractere;
    }
    if (caractere == EOF && longueur == 0) {
        free(ligne);
        return NULL;
    }
    ligne[longueur] = '\0';
    return ligne;
}

static size_t compter_occurrences(const char *ligne, const char *phrase)
{
    size_t nombre = 0;
    size_t longueur_phrase = strlen(phrase);

    for (size_t i = 0; ligne[i] != '\0'; ++i) {
        size_t j = 0;
        while (j < longueur_phrase && ligne[i + j] == phrase[j]) {
            ++j;
        }
        if (j == longueur_phrase) {
            ++nombre;
        }
    }
    return nombre;
}

int main(int argc, char **argv)
{
    char nom_fichier[512];
    char phrase[512];
    FILE *fichier;
    size_t numero_ligne = 0;
    int resultats = 0;
    int erreur_lecture = 0;
    int erreur_allocation = 0;

    if (argc > 2) {
        fprintf(stderr, "Usage : %s [fichier]\n", argv[0]);
        return 1;
    }
    if (argc == 2) {
        if (strlen(argv[1]) >= sizeof nom_fichier) {
            fprintf(stderr, "Nom de fichier trop long.\n");
            return 1;
        }
        strcpy(nom_fichier, argv[1]);
    } else {
        printf("Entrez le nom du fichier : ");
        if (fgets(nom_fichier, sizeof nom_fichier, stdin) == NULL) {
            fprintf(stderr, "Impossible de lire le nom du fichier.\n");
            return 1;
        }
        nom_fichier[strcspn(nom_fichier, "\n")] = '\0';
    }

    printf("Entrez la phrase que vous souhaitez rechercher : ");
    if (fgets(phrase, sizeof phrase, stdin) == NULL) {
        fprintf(stderr, "Impossible de lire la phrase.\n");
        return 1;
    }
    phrase[strcspn(phrase, "\n")] = '\0';
    if (phrase[0] == '\0') {
        fprintf(stderr, "La phrase a rechercher ne peut pas etre vide.\n");
        return 1;
    }

    fichier = fopen(nom_fichier, "r");
    if (fichier == NULL) {
        perror(nom_fichier);
        return 1;
    }
    printf("Resultats de la recherche :\n");
    for (;;) {
        char *ligne = lire_ligne(fichier, &erreur_allocation);
        size_t occurrences;

        if (ligne == NULL) {
            if (erreur_allocation || ferror(fichier)) {
                erreur_lecture = 1;
            }
            break;
        }
        ++numero_ligne;
        occurrences = compter_occurrences(ligne, phrase);
        if (occurrences > 0) {
            printf("Ligne %zu, %zu fois\n", numero_ligne, occurrences);
            resultats = 1;
        }
        free(ligne);
    }
    if (fclose(fichier) == EOF) {
        perror("Erreur de fermeture");
        return 1;
    }
    if (erreur_lecture) {
        perror("Erreur de lecture");
        return 1;
    }
    if (!resultats) {
        printf("Phrase absente.\n");
    }
    return 0;
}
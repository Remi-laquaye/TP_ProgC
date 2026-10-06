#define _POSIX_C_SOURCE 200809L

#include "repertoire.h"

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static char *joindre_chemin(const char *repertoire, const char *nom)
{
    size_t longueur_repertoire = strlen(repertoire);
    size_t longueur_nom = strlen(nom);
    int separateur = longueur_repertoire > 0 &&
                     repertoire[longueur_repertoire - 1] != '/';
    char *chemin = malloc(longueur_repertoire + (size_t)separateur +
                          longueur_nom + 1);

    if (chemin == NULL) {
        perror("Allocation du chemin");
        return NULL;
    }
    memcpy(chemin, repertoire, longueur_repertoire);
    if (separateur) {
        chemin[longueur_repertoire++] = '/';
    }
    memcpy(chemin + longueur_repertoire, nom, longueur_nom + 1);
    return chemin;
}

static int est_repertoire(const char *chemin)
{
    struct stat informations;

    if (lstat(chemin, &informations) < 0) {
        perror(chemin);
        return 0;
    }
    return S_ISDIR(informations.st_mode);
}

int lire_dossier(const char *nom_repertoire)
{
    DIR *dossier = opendir(nom_repertoire);
    struct dirent *entree;
    int erreur = 0;

    if (dossier == NULL) {
        perror(nom_repertoire);
        return -1;
    }
    while (1) {
        errno = 0;
        entree = readdir(dossier);
        if (entree == NULL) {
            if (errno != 0) {
                perror(nom_repertoire);
                erreur = 1;
            }
            break;
        }
        if (strcmp(entree->d_name, ".") != 0 &&
            strcmp(entree->d_name, "..") != 0) {
            printf("%s\n", entree->d_name);
        }
    }
    if (closedir(dossier) < 0) {
        perror(nom_repertoire);
        erreur = 1;
    }
    return erreur ? -1 : 0;
}

int lire_dossier_recursif(const char *nom_repertoire)
{
    DIR *dossier = opendir(nom_repertoire);
    struct dirent *entree;
    int erreur = 0;

    if (dossier == NULL) {
        perror(nom_repertoire);
        return -1;
    }
    while (1) {
        char *chemin;

        errno = 0;
        entree = readdir(dossier);
        if (entree == NULL) {
            if (errno != 0) {
                perror(nom_repertoire);
                erreur = 1;
            }
            break;
        }
        if (strcmp(entree->d_name, ".") == 0 ||
            strcmp(entree->d_name, "..") == 0) {
            continue;
        }
        chemin = joindre_chemin(nom_repertoire, entree->d_name);
        if (chemin == NULL) {
            erreur = 1;
            continue;
        }
        printf("%s\n", chemin);
        if (est_repertoire(chemin) && lire_dossier_recursif(chemin) < 0) {
            erreur = 1;
        }
        free(chemin);
    }
    if (closedir(dossier) < 0) {
        perror(nom_repertoire);
        erreur = 1;
    }
    return erreur ? -1 : 0;
}

int lire_dossier_iteratif(const char *nom_repertoire)
{
    size_t capacite = 16;
    size_t nombre = 1;
    char **pile = malloc(capacite * sizeof *pile);
    int erreur = 0;

    if (pile == NULL) {
        perror("Allocation de la pile des repertoires");
        return -1;
    }
    pile[0] = strdup(nom_repertoire);
    if (pile[0] == NULL) {
        perror("Allocation du chemin");
        free(pile);
        return -1;
    }

    while (nombre > 0) {
        char *repertoire = pile[--nombre];
        DIR *dossier = opendir(repertoire);
        struct dirent *entree;

        if (dossier == NULL) {
            perror(repertoire);
            free(repertoire);
            erreur = 1;
            continue;
        }
        while (1) {
            char *chemin;

            errno = 0;
            entree = readdir(dossier);
            if (entree == NULL) {
                if (errno != 0) {
                    perror(repertoire);
                    erreur = 1;
                }
                break;
            }
            if (strcmp(entree->d_name, ".") == 0 ||
                strcmp(entree->d_name, "..") == 0) {
                continue;
            }
            chemin = joindre_chemin(repertoire, entree->d_name);
            if (chemin == NULL) {
                erreur = 1;
                continue;
            }
            printf("%s\n", chemin);
            if (est_repertoire(chemin)) {
                if (nombre == capacite) {
                    size_t nouvelle_capacite = capacite * 2;
                    char **nouvelle_pile =
                        realloc(pile, nouvelle_capacite * sizeof *pile);
                    if (nouvelle_pile == NULL) {
                        perror("Agrandissement de la pile");
                        free(chemin);
                        erreur = 1;
                        continue;
                    }
                    pile = nouvelle_pile;
                    capacite = nouvelle_capacite;
                }
                pile[nombre++] = chemin;
            } else {
                free(chemin);
            }
        }
        if (closedir(dossier) < 0) {
            perror(repertoire);
            erreur = 1;
        }
        free(repertoire);
    }
    free(pile);
    return erreur ? -1 : 0;
}

int main(int argc, char **argv)
{
    const char *repertoire;
    int resultat;

    if (argc == 2) {
        repertoire = argv[1];
        resultat = lire_dossier(repertoire);
    } else if (argc == 3 && strcmp(argv[1], "-r") == 0) {
        repertoire = argv[2];
        resultat = lire_dossier_recursif(repertoire);
    } else if (argc == 3 && strcmp(argv[1], "-i") == 0) {
        repertoire = argv[2];
        resultat = lire_dossier_iteratif(repertoire);
    } else {
        fprintf(stderr, "Usage : %s [-r|-i] <repertoire>\n", argv[0]);
        return EXIT_FAILURE;
    }
    return resultat == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
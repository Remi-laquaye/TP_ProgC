#include "couleur.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    uint32_t cle;
    size_t compte;
} entree_couleur;

static size_t hacher(uint32_t cle)
{
    cle ^= cle >> 16;
    cle *= UINT32_C(0x7feb352d);
    cle ^= cle >> 15;
    cle *= UINT32_C(0x846ca68b);
    cle ^= cle >> 16;
    return (size_t)cle;
}

static int comparer24(const void *gauche, const void *droite)
{
    const couleur24_compteur *a = gauche;
    const couleur24_compteur *b = droite;

    if (a->compte < b->compte) {
        return 1;
    }
    if (a->compte > b->compte) {
        return -1;
    }
    if (a->c.rouge != b->c.rouge) {
        return (int)a->c.rouge - (int)b->c.rouge;
    }
    if (a->c.vert != b->c.vert) {
        return (int)a->c.vert - (int)b->c.vert;
    }
    return (int)a->c.bleu - (int)b->c.bleu;
}

static int comparer32(const void *gauche, const void *droite)
{
    const couleur32_compteur *a = gauche;
    const couleur32_compteur *b = droite;

    if (a->compte < b->compte) {
        return 1;
    }
    if (a->compte > b->compte) {
        return -1;
    }
    if (a->c.rouge != b->c.rouge) {
        return (int)a->c.rouge - (int)b->c.rouge;
    }
    if (a->c.vert != b->c.vert) {
        return (int)a->c.vert - (int)b->c.vert;
    }
    if (a->c.bleu != b->c.bleu) {
        return (int)a->c.bleu - (int)b->c.bleu;
    }
    return (int)a->c.alpha - (int)b->c.alpha;
}

couleur_compteur *compte_couleur(const couleur *image, size_t nombre_pixels)
{
    couleur_compteur *resultat;
    entree_couleur *entrees;
    size_t capacite_entrees = 128;
    size_t nombre_entrees = 0;
    size_t capacite_table = 256;
    size_t *table;

    if (image == NULL || nombre_pixels == 0 ||
        (image->compte_bit != BITS24 && image->compte_bit != BITS32)) {
        return NULL;
    }
    if ((image->compte_bit == BITS24 && image->c.c24 == NULL) ||
        (image->compte_bit == BITS32 && image->c.c32 == NULL)) {
        return NULL;
    }
    entrees = malloc(capacite_entrees * sizeof *entrees);
    table = calloc(capacite_table, sizeof *table);
    if (entrees == NULL || table == NULL) {
        perror("Allocation de la table des couleurs");
        free(entrees);
        free(table);
        return NULL;
    }

    for (size_t i = 0; i < nombre_pixels; ++i) {
        uint32_t cle;
        size_t emplacement;

        if (image->compte_bit == BITS24) {
            const couleur24 *pixel = &image->c.c24[i];
            cle = ((uint32_t)pixel->rouge << 16) |
                  ((uint32_t)pixel->vert << 8) | (uint32_t)pixel->bleu;
        } else {
            const couleur32 *pixel = &image->c.c32[i];
            cle = ((uint32_t)pixel->rouge << 24) |
                  ((uint32_t)pixel->vert << 16) |
                  ((uint32_t)pixel->bleu << 8) | (uint32_t)pixel->alpha;
        }
        emplacement = hacher(cle) & (capacite_table - 1);
        while (table[emplacement] != 0 &&
               entrees[table[emplacement] - 1].cle != cle) {
            emplacement = (emplacement + 1) & (capacite_table - 1);
        }
        if (table[emplacement] != 0) {
            ++entrees[table[emplacement] - 1].compte;
            continue;
        }

        if (nombre_entrees == capacite_entrees) {
            size_t nouvelle_capacite;
            entree_couleur *nouveaux;

            if (capacite_entrees > SIZE_MAX / 2 / sizeof *entrees) {
                fprintf(stderr, "Trop de couleurs distinctes.\n");
                free(entrees);
                free(table);
                return NULL;
            }
            nouvelle_capacite = capacite_entrees * 2;
            nouveaux = realloc(entrees, nouvelle_capacite * sizeof *entrees);
            if (nouveaux == NULL) {
                perror("Agrandissement des couleurs distinctes");
                free(entrees);
                free(table);
                return NULL;
            }
            entrees = nouveaux;
            capacite_entrees = nouvelle_capacite;
        }
        entrees[nombre_entrees].cle = cle;
        entrees[nombre_entrees].compte = 1;
        table[emplacement] = ++nombre_entrees;

        if (nombre_entrees >= capacite_table - capacite_table / 3) {
            size_t nouvelle_capacite;
            size_t *nouvelle_table;

            if (capacite_table > SIZE_MAX / 2 / sizeof *table) {
                fprintf(stderr, "Table des couleurs trop grande.\n");
                free(entrees);
                free(table);
                return NULL;
            }
            nouvelle_capacite = capacite_table * 2;
            nouvelle_table = calloc(nouvelle_capacite,
                                    sizeof *nouvelle_table);
            if (nouvelle_table == NULL) {
                perror("Agrandissement de la table des couleurs");
                free(entrees);
                free(table);
                return NULL;
            }
            for (size_t j = 0; j < nombre_entrees; ++j) {
                size_t index = hacher(entrees[j].cle) &
                               (nouvelle_capacite - 1);
                while (nouvelle_table[index] != 0) {
                    index = (index + 1) & (nouvelle_capacite - 1);
                }
                nouvelle_table[index] = j + 1;
            }
            free(table);
            table = nouvelle_table;
            capacite_table = nouvelle_capacite;
        }
    }

    resultat = calloc(1, sizeof *resultat);
    if (resultat == NULL) {
        perror("Allocation du compteur de couleurs");
        free(entrees);
        free(table);
        return NULL;
    }
    resultat->compte_bit = image->compte_bit;

    if (image->compte_bit == BITS24) {
        resultat->cc.cc24 = calloc(nombre_entrees,
                                    sizeof *resultat->cc.cc24);
        if (resultat->cc.cc24 == NULL) {
            perror("Allocation des couleurs distinctes");
            free(resultat);
            free(entrees);
            free(table);
            return NULL;
        }
        for (size_t i = 0; i < nombre_entrees; ++i) {
            resultat->cc.cc24[i].c.rouge = (uint8_t)(entrees[i].cle >> 16);
            resultat->cc.cc24[i].c.vert = (uint8_t)(entrees[i].cle >> 8);
            resultat->cc.cc24[i].c.bleu = (uint8_t)entrees[i].cle;
            resultat->cc.cc24[i].compte = entrees[i].compte;
        }
    } else {
        resultat->cc.cc32 = calloc(nombre_entrees,
                                    sizeof *resultat->cc.cc32);
        if (resultat->cc.cc32 == NULL) {
            perror("Allocation des couleurs distinctes");
            free(resultat);
            free(entrees);
            free(table);
            return NULL;
        }
        for (size_t i = 0; i < nombre_entrees; ++i) {
            resultat->cc.cc32[i].c.rouge = (uint8_t)(entrees[i].cle >> 24);
            resultat->cc.cc32[i].c.vert = (uint8_t)(entrees[i].cle >> 16);
            resultat->cc.cc32[i].c.bleu = (uint8_t)(entrees[i].cle >> 8);
            resultat->cc.cc32[i].c.alpha = (uint8_t)entrees[i].cle;
            resultat->cc.cc32[i].compte = entrees[i].compte;
        }
    }
    resultat->size = nombre_entrees;
    free(entrees);
    free(table);
    return resultat;
}

void print_couleur(const couleur *image, size_t nombre)
{
    if (image == NULL) {
        return;
    }
    for (size_t i = 0; i < nombre; ++i) {
        if (image->compte_bit == BITS24) {
            printf("%02x %02x %02x\n", image->c.c24[i].rouge,
                   image->c.c24[i].vert, image->c.c24[i].bleu);
        } else if (image->compte_bit == BITS32) {
            printf("%02x %02x %02x %02x\n", image->c.c32[i].rouge,
                   image->c.c32[i].vert, image->c.c32[i].bleu,
                   image->c.c32[i].alpha);
        }
    }
}

void trier_couleur_compteur(couleur_compteur *compteur)
{
    if (compteur == NULL) {
        return;
    }
    if (compteur->compte_bit == BITS24) {
        qsort(compteur->cc.cc24, compteur->size,
              sizeof *compteur->cc.cc24, comparer24);
    } else if (compteur->compte_bit == BITS32) {
        qsort(compteur->cc.cc32, compteur->size,
              sizeof *compteur->cc.cc32, comparer32);
    }
}

void print_couleur_compteur(const couleur_compteur *compteur)
{
    if (compteur == NULL) {
        return;
    }
    for (size_t i = 0; i < compteur->size; ++i) {
        if (compteur->compte_bit == BITS24) {
            const couleur24_compteur *c = &compteur->cc.cc24[i];
            printf("#%02x%02x%02x: %zu\n", c->c.rouge, c->c.vert,
                   c->c.bleu, c->compte);
        } else if (compteur->compte_bit == BITS32) {
            const couleur32_compteur *c = &compteur->cc.cc32[i];
            printf("#%02x%02x%02x: %zu\n", c->c.rouge, c->c.vert,
                   c->c.bleu, c->compte);
        }
    }
}

void liberer_couleur_compteur(couleur_compteur *compteur)
{
    if (compteur == NULL) {
        return;
    }
    if (compteur->compte_bit == BITS24) {
        free(compteur->cc.cc24);
    } else if (compteur->compte_bit == BITS32) {
        free(compteur->cc.cc32);
    }
    free(compteur);
}

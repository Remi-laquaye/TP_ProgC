#ifndef COULEUR_H
#define COULEUR_H

#include <stddef.h>
#include <stdint.h>

#define MAX_COULEURS 30

typedef enum {
    BITS24 = 24,
    BITS32 = 32
} COMPTEBIT;

typedef struct {
    uint8_t bleu;
    uint8_t vert;
    uint8_t rouge;
    uint8_t alpha;
} couleur32;

typedef struct {
    uint8_t bleu;
    uint8_t vert;
    uint8_t rouge;
} couleur24;

typedef struct {
    COMPTEBIT compte_bit;
    union {
        couleur24 *c24;
        couleur32 *c32;
    } c;
    size_t size;
} couleur;

typedef struct {
    couleur32 c;
    size_t compte;
} couleur32_compteur;

typedef struct {
    couleur24 c;
    size_t compte;
} couleur24_compteur;

typedef struct {
    COMPTEBIT compte_bit;
    union {
        couleur24_compteur *cc24;
        couleur32_compteur *cc32;
    } cc;
    size_t size;
} couleur_compteur;

couleur_compteur *compte_couleur(const couleur *image, size_t nombre_pixels);
void print_couleur(const couleur *image, size_t nombre);
void print_couleur_compteur(const couleur_compteur *compteur);
void trier_couleur_compteur(couleur_compteur *compteur);
void liberer_couleur_compteur(couleur_compteur *compteur);

#endif

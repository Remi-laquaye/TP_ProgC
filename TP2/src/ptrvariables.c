#include <stddef.h>
#include <stdio.h>

static void afficher_hexadecimal(const char *nom, const void *adresse,
                                 size_t taille)
{
    const unsigned char *octets = adresse;

    printf("Adresse de %s : %p, valeur (octets hex) : ", nom, adresse);
    for (size_t i = 0; i < taille; ++i) {
        printf("%02x", (unsigned int)octets[i]);
    }
    printf("\n");
}

int main(void)
{
    char caractere = 'A';
    short court = 1234;
    int entier = 123456;
    long grand_entier = 123456789L;
    long long tres_grand_entier = 1234567890123LL;
    float reel = 3.25f;
    double double_precision = 6.5;
    long double longue_precision = 9.75L;

    char *p_caractere = &caractere;
    short *p_court = &court;
    int *p_entier = &entier;
    long *p_grand_entier = &grand_entier;
    long long *p_tres_grand_entier = &tres_grand_entier;
    float *p_reel = &reel;
    double *p_double_precision = &double_precision;
    long double *p_longue_precision = &longue_precision;

    printf("Avant la manipulation :\n");
    afficher_hexadecimal("caractere", p_caractere, sizeof *p_caractere);
    afficher_hexadecimal("court", p_court, sizeof *p_court);
    afficher_hexadecimal("entier", p_entier, sizeof *p_entier);
    afficher_hexadecimal("grand_entier", p_grand_entier,
                         sizeof *p_grand_entier);
    afficher_hexadecimal("tres_grand_entier", p_tres_grand_entier,
                         sizeof *p_tres_grand_entier);
    afficher_hexadecimal("reel", p_reel, sizeof *p_reel);
    afficher_hexadecimal("double_precision", p_double_precision,
                         sizeof *p_double_precision);
    afficher_hexadecimal("longue_precision", p_longue_precision,
                         sizeof *p_longue_precision);

    ++*p_caractere;
    ++*p_court;
    ++*p_entier;
    ++*p_grand_entier;
    ++*p_tres_grand_entier;
    *p_reel += 1.0f;
    *p_double_precision += 1.0;
    *p_longue_precision += 1.0L;

    printf("\nApres la manipulation :\n");
    afficher_hexadecimal("caractere", p_caractere, sizeof *p_caractere);
    afficher_hexadecimal("court", p_court, sizeof *p_court);
    afficher_hexadecimal("entier", p_entier, sizeof *p_entier);
    afficher_hexadecimal("grand_entier", p_grand_entier,
                         sizeof *p_grand_entier);
    afficher_hexadecimal("tres_grand_entier", p_tres_grand_entier,
                         sizeof *p_tres_grand_entier);
    afficher_hexadecimal("reel", p_reel, sizeof *p_reel);
    afficher_hexadecimal("double_precision", p_double_precision,
                         sizeof *p_double_precision);
    afficher_hexadecimal("longue_precision", p_longue_precision,
                         sizeof *p_longue_precision);
    return 0;
}
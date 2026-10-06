#include <stdio.h>
#include <stddef.h>

static int lire_ligne(const char *invite, char *destination, size_t capacite)
{
    size_t longueur = 0;
    int caractere;
    int trop_longue = 0;

    printf("%s", invite);
    while ((caractere = getchar()) != '\n' && caractere != EOF) {
        if (longueur + 1 < capacite) {
            destination[longueur++] = (char)caractere;
        } else {
            trop_longue = 1;
        }
    }
    destination[longueur] = '\0';

    if (trop_longue || (caractere == EOF && longueur == 0)) {
        fprintf(stderr, "Ligne trop longue ou fin de saisie inattendue.\n");
        return 0;
    }
    return 1;
}

static size_t longueur_chaine(const char *chaine)
{
    size_t longueur = 0;
    while (chaine[longueur] != '\0') {
        ++longueur;
    }
    return longueur;
}

static void copier_chaine(char *destination, const char *source)
{
    while ((*destination++ = *source++) != '\0') {
    }
}

static int concatener_chaine(char *destination, const char *source,
                             size_t capacite)
{
    size_t fin = longueur_chaine(destination);
    size_t i = 0;

    while (source[i] != '\0') {
        if (fin + i + 1 >= capacite) {
            return 0;
        }
        destination[fin + i] = source[i];
        ++i;
    }
    destination[fin + i] = '\0';
    return 1;
}

int main(void)
{
    char premiere[256];
    char seconde[256];
    char copie[256];
    char concatenee[512];

    if (!lire_ligne("Premiere chaine : ", premiere, sizeof premiere) ||
        !lire_ligne("Deuxieme chaine : ", seconde, sizeof seconde)) {
        return 1;
    }

    copier_chaine(copie, premiere);
    copier_chaine(concatenee, premiere);
    if (!concatener_chaine(concatenee, seconde, sizeof concatenee)) {
        fprintf(stderr, "La chaine concatenee est trop longue.\n");
        return 1;
    }

    printf("Longueur de la premiere chaine : %zu\n",
           longueur_chaine(premiere));
    printf("Longueur de la deuxieme chaine : %zu\n",
           longueur_chaine(seconde));
    printf("Longueur totale : %zu\n", longueur_chaine(concatenee));
    printf("Copie : %s\n", copie);
    printf("Concatenee : %s\n", concatenee);
    return 0;
}
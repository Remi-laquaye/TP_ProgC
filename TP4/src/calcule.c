#include "operator.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

static int convertir_entier(const char *texte, int *valeur)
{
    char *fin;
    long resultat;

    errno = 0;
    resultat = strtol(texte, &fin, 10);
    if (texte == fin || *fin != '\0' || errno == ERANGE ||
        resultat < INT_MIN || resultat > INT_MAX) {
        return 0;
    }
    *valeur = (int)resultat;
    return 1;
}

int main(int argc, char **argv)
{
    int num1;
    int num2;
    int resultat;

    if (argc != 4 || argv[1][0] == '\0' || argv[1][1] != '\0' ||
        !convertir_entier(argv[2], &num1) ||
        !convertir_entier(argv[3], &num2)) {
        fprintf(stderr, "Usage : %s <+|-|*|/|%%|&|||~> <entier> <entier>\n",
                argv[0]);
        return 1;
    }
    if (!operation(argv[1][0], num1, num2, &resultat)) {
        fprintf(stderr, "Operateur invalide ou operation impossible.\n");
        return 1;
    }
    printf("Resultat : %d\n", resultat);
    return 0;
}
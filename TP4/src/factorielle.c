#include <stdio.h>

static unsigned long long factorielle(unsigned int nombre)
{
    unsigned long long resultat;

    if (nombre == 0) {
        printf("fact(0): 1\n");
        return 1;
    }
    resultat = nombre * factorielle(nombre - 1);
    printf("fact(%u): %llu\n", nombre, resultat);
    return resultat;
}

int main(void)
{
    const unsigned int valeurs[] = {0, 1, 5, 10, 20};

    for (size_t i = 0; i < sizeof valeurs / sizeof valeurs[0]; ++i) {
        printf("%u! = %llu\n\n", valeurs[i], factorielle(valeurs[i]));
    }
    return 0;
}
#include <stdio.h>

int main(void)
{
    unsigned int n;
    unsigned long long precedent = 0;
    unsigned long long courant = 1;

    printf("Saisissez n (0 a 93) : ");
    if (scanf("%u", &n) != 1 || n > 93) {
        fprintf(stderr, "Veuillez saisir un entier compris entre 0 et 93.\n");
        return 1;
    }

    for (unsigned int i = 0; i <= n; ++i) {
        printf("%llu", precedent);
        if (i < n) {
            printf(", ");
            unsigned long long suivant = precedent + courant;
            precedent = courant;
            courant = suivant;
        }
    }
    printf("\n");
    return 0;
}
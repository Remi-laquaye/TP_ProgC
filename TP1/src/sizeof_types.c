#include <stdio.h>

int main(void) {
    printf("=== TAILLE DES TYPES EN OCTETS ===\n\n");

    /* Types entiers : char */
    printf("char : %zu octet(s)\n", sizeof(char));
    printf("signed char : %zu octet(s)\n", sizeof(signed char));
    printf("unsigned char : %zu octet(s)\n\n", sizeof(unsigned char));

    /*Types entiers : short */
    printf("short : %zu octet(s)\n", sizeof(short));
    printf("signed short : %zu octet(s)\n", sizeof(signed short));
    printf("unsigned short : %zu octet(s)\n\n", sizeof(unsigned short));

    /* Types entiers : int */
    printf("int : %zu octet(s)\n", sizeof(int));
    printf("signed int : %zu octet(s)\n", sizeof(signed int));
    printf("unsigned int : %zu octet(s)\n\n", sizeof(unsigned int));

    /* Types entiers : long int */
    printf("long int : %zu octet(s)\n", sizeof(long int));
    printf("signed long int : %zu octet(s)\n", sizeof(signed long int));
    printf("unsigned long int : %zu octet(s)\n\n", sizeof(unsigned long int));

    /* Types entiers : long long int */
    printf("long long int : %zu octet(s)\n", sizeof(long long int));
    printf("signed long long int : %zu octet(s)\n", sizeof(signed long long int));
    printf("unsigned long long int : %zu octet(s)\n\n", sizeof(unsigned long long int));

    /*Types flottants (pas de signed/unsigned) */
    printf("float : %zu octet(s)\n", sizeof(float));
    printf("double : %zu octet(s)\n", sizeof(double));
    printf("long double : %zu octet(s)\n", sizeof(long double));

    return 0;
}
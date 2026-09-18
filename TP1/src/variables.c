#include <stdio.h>

int main(void) {
    /*Types entiers signés et non signés */
    signed char sc = -42;
    unsigned char uc = 200;

    signed short ss = -12345;
    unsigned short us = 54321;

    signed int si = -100000;
    unsigned int ui = 3000000000U;

    signed long int sli = -1234567890L;
    unsigned long int uli = 3456789012UL;

    signed long long int slli = -9000000000000000000LL;
    unsigned long long int ulli = 18000000000000000000ULL;

    /* Types réels à virgule flottante */
    float f = 3.14159f;
    double d = 2.718281828459;
    long double ld = 1.4142135623730950488L;

    /* Affichage des valeurs */
    printf("signed char : %d\n", sc);
    printf("unsigned char : %u\n", uc);

    printf("signed short : %d\n", ss);
    printf("unsigned short : %u\n", us);

    printf("signed int : %d\n", si);
    printf("unsigned int : %u\n", ui);

    printf("signed long int : %ld\n", sli);
    printf("unsigned long int : %lu\n", uli);

    printf("signed long long int : %lld\n", slli);
    printf("unsigned long long int : %llu\n", ulli);

    printf("float : %f\n", f);
    printf("double : %lf\n", d);
    printf("long double : %Lf\n", ld);

    return 0;
}
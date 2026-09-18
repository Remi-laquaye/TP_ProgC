#include <stdio.h>
#include <stdbool.h>

int main(void) {
    int a = 16;
    int b = 3;

    printf("=== VALEURS DE DÉPART ===\n");
    printf("a = %d, b = %d\n\n", a, b);

    /* 1. Addition */
    printf("1. Addition (a + b) : %d\n", a + b);

    /* 2. Soustraction */
    printf("2. Soustraction (a - b) : %d\n", a - b);

    /* 3. Multiplication */
    printf("3. Multiplication (a b) : %d\n", a * b);

    /* 4. Division entière */
    printf("4. Division (a / b) : %d\n", a / b);

    /* 5. Modulo (reste de la division) */
    printf("5. Modulo (a %% b) : %d\n\n", a % b);

    /* 6. Égalité (a == b) */
    bool est_egal = (a == b);
    printf("6. Egalite (a == b) : %s (valeur numérique : %d)\n", est_egal ? "true" : "false", est_egal);

    /* 7. Supériorité (a > b) */
    bool est_superieur = (a > b);
    printf("7. Superieur (a > b) : %s (valeur numérique : %d)\n", est_superieur ? "true" : "false", est_superieur);

    return 0;
}
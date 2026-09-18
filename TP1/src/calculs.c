#include <stdio.h>

int main(void) {
    int num1 = 12;
    int num2 = 5;
    char op = '&'; // Tester avec : '+', '-', '', '/', '%', '&', '|', '~'

    printf("=== CALCULATRICE (num1 = %d, num2 = %d, op = '%c') ===\n\n", num1, num2, op);

    switch (op) {
        /* Opérations arithmétiques */
        case '+':
            printf("%d + %d = %d\n", num1, num2, num1 + num2);
            break;

        case '-':
            printf("%d - %d = %d\n", num1, num2, num1 - num2);
            break;

        case '*':
            printf("%d * %d = %d\n", num1, num2, num1 * num2);
            break;

        case '/':
            if (num2 != 0) {
                printf("%d / %d = %d\n", num1, num2, num1 / num2);
            } else {
                printf("Erreur : division par zero impossible !\n");
            }
            break;

        case '%':
            if (num2 != 0) {
                printf("%d %% %d = %d\n", num1, num2, num1 % num2);
            } else {
                printf("Erreur : modulo par zero impossible !\n");
            }
            break;

        /* Opérations bit à bit (bitwise) */
        case '&':
            printf("%d & %d (ET bit a bit) = %d\n", num1, num2, num1 & num2);
            break;

        case '|':
            printf("%d | %d (OU bit a bit) = %d\n", num1, num2, num1 | num2);
            break;

        case '~':
            /* L'opérateur NON (bitwise NOT) est unaire : il s'applique à un seul opérande */
            printf("~%d (NOT bit a bit sur num1) = %d\n", num1, ~num1);
            printf("~%d (NOT bit a bit sur num2) = %d\n", num2, ~num2);
            break;

        default:
            printf("Erreur : operateur '%c' non reconnu.\n", op);
            break;
    }

    return 0;
}
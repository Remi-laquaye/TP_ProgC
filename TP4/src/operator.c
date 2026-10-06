#include "operator.h"

#include <limits.h>

int operation(char operateur, int num1, int num2, int *resultat)
{
    long long valeur;

    if (resultat == 0) {
        return 0;
    }

    switch (operateur) {
    case '+':
        valeur = (long long)num1 + num2;
        break;
    case '-':
        valeur = (long long)num1 - num2;
        break;
    case '*':
        valeur = (long long)num1 * num2;
        break;
    case '/':
        if (num2 == 0 || (num1 == INT_MIN && num2 == -1)) {
            return 0;
        }
        *resultat = num1 / num2;
        return 1;
    case '%':
        if (num2 == 0 || (num1 == INT_MIN && num2 == -1)) {
            return 0;
        }
        *resultat = num1 % num2;
        return 1;
    case '&':
        *resultat = num1 & num2;
        return 1;
    case '|':
        *resultat = num1 | num2;
        return 1;
    case '~':
        *resultat = ~num1;
        return 1;
    default:
        return 0;
    }

    if (valeur < INT_MIN || valeur > INT_MAX) {
        return 0;
    }
    *resultat = (int)valeur;
    return 1;
}
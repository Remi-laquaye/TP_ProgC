#include <stdio.h>

#define LONGUEUR_MAX 256

int main(void)
{
    const char *phrases[10] = {
        "Bonjour, comment ca va ?",
        "Le temps est magnifique aujourd'hui.",
        "C'est une belle journee.",
        "La programmation en C est amusante.",
        "Les tableaux en C sont puissants.",
        "Les pointeurs en C peuvent etre deroutants.",
        "Il fait beau dehors.",
        "La recherche dans un tableau est interessante.",
        "Les structures de donnees sont importantes.",
        "Programmer en C, c'est genial."
    };
    char recherche[LONGUEUR_MAX];
    int trouve = 0;
    int caractere;
    size_t longueur = 0;
    int trop_longue = 0;

    printf("Entrez la phrase a rechercher : ");
    while ((caractere = getchar()) != '\n' && caractere != EOF) {
        if (longueur + 1 < sizeof recherche) {
            recherche[longueur++] = (char)caractere;
        } else {
            trop_longue = 1;
        }
    }
    recherche[longueur] = '\0';
    if (trop_longue || (caractere == EOF && longueur == 0)) {
        fprintf(stderr, "Phrase trop longue ou fin de saisie inattendue.\n");
        return 1;
    }

    for (int i = 0; i < 10; ++i) {
        size_t j = 0;
        while (phrases[i][j] != '\0' && recherche[j] != '\0' &&
               phrases[i][j] == recherche[j]) {
            ++j;
        }
        if (phrases[i][j] == '\0' && recherche[j] == '\0') {
            trouve = 1;
            break;
        }
    }

    printf("%s\n", trouve ? "Phrase trouvee" : "Phrase non trouvee");
    return 0;
}

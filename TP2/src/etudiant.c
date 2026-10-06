#include <stdio.h>

int main(void)
{
    const char *noms[5] = {
        "Dupont", "Martin", "Bernard", "Petit", "Robert"
    };
    const char *prenoms[5] = {
        "Marie", "Pierre", "Sofia", "Lucas", "Emma"
    };
    const char *adresses[5] = {
        "20, Boulevard Niels Bohr, Lyon",
        "22, Boulevard Niels Bohr, Lyon",
        "5, rue de la Paix, Paris",
        "12, avenue des Fleurs, Nice",
        "8, place du Marche, Lille"
    };
    const float notes_c[5] = {16.5f, 14.0f, 18.0f, 12.5f, 15.0f};
    const float notes_systeme[5] = {12.1f, 14.1f, 17.0f, 13.5f, 16.0f};

    for (int i = 0; i < 5; ++i) {
        printf("Etudiant.e %d :\n", i + 1);
        printf("Nom : %s\n", noms[i]);
        printf("Prenom : %s\n", prenoms[i]);
        printf("Adresse : %s\n", adresses[i]);
        printf("Programmation en C : %.1f\n", notes_c[i]);
        printf("Systeme d'exploitation : %.1f\n\n", notes_systeme[i]);
    }
    return 0;
}
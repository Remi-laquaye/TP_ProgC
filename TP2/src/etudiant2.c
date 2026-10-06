#include <stdio.h>
#include <string.h>

typedef struct {
    char nom[32];
    char prenom[32];
    char adresse[96];
    float note_c;
    float note_systeme;
} Etudiant;

int main(void)
{
    Etudiant etudiants[5];

    strcpy(etudiants[0].nom, "Dupont");
    strcpy(etudiants[0].prenom, "Marie");
    strcpy(etudiants[0].adresse, "20, Boulevard Niels Bohr, Lyon");
    etudiants[0].note_c = 16.5f;
    etudiants[0].note_systeme = 12.1f;

    strcpy(etudiants[1].nom, "Martin");
    strcpy(etudiants[1].prenom, "Pierre");
    strcpy(etudiants[1].adresse, "22, Boulevard Niels Bohr, Lyon");
    etudiants[1].note_c = 14.0f;
    etudiants[1].note_systeme = 14.1f;

    strcpy(etudiants[2].nom, "Bernard");
    strcpy(etudiants[2].prenom, "Sofia");
    strcpy(etudiants[2].adresse, "5, rue de la Paix, Paris");
    etudiants[2].note_c = 18.0f;
    etudiants[2].note_systeme = 17.0f;

    strcpy(etudiants[3].nom, "Petit");
    strcpy(etudiants[3].prenom, "Lucas");
    strcpy(etudiants[3].adresse, "12, avenue des Fleurs, Nice");
    etudiants[3].note_c = 12.5f;
    etudiants[3].note_systeme = 13.5f;

    strcpy(etudiants[4].nom, "Robert");
    strcpy(etudiants[4].prenom, "Emma");
    strcpy(etudiants[4].adresse, "8, place du Marche, Lille");
    etudiants[4].note_c = 15.0f;
    etudiants[4].note_systeme = 16.0f;

    for (int i = 0; i < 5; ++i) {
        printf("Etudiant.e %d :\n", i + 1);
        printf("Nom : %s\n", etudiants[i].nom);
        printf("Prenom : %s\n", etudiants[i].prenom);
        printf("Adresse : %s\n", etudiants[i].adresse);
        printf("Programmation en C : %.1f\n", etudiants[i].note_c);
        printf("Systeme d'exploitation : %.1f\n\n",
               etudiants[i].note_systeme);
    }
    return 0;
}
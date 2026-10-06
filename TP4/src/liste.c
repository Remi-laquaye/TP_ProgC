#include "liste.h"

#include <stdio.h>
#include <stdlib.h>

void init_liste(struct liste_couleurs *liste)
{
    if (liste != NULL) {
        liste->tete = NULL;
    }
}

int insertion(const struct couleur *couleur, struct liste_couleurs *liste)
{
    struct noeud_couleur *nouveau;

    if (couleur == NULL || liste == NULL) {
        return 0;
    }
    nouveau = malloc(sizeof *nouveau);
    if (nouveau == NULL) {
        perror("Allocation d'un noeud");
        return 0;
    }
    nouveau->valeur = *couleur;
    nouveau->suivant = liste->tete;
    liste->tete = nouveau;
    return 1;
}

void parcours(const struct liste_couleurs *liste)
{
    const struct noeud_couleur *courant;

    if (liste == NULL) {
        return;
    }
    for (courant = liste->tete; courant != NULL; courant = courant->suivant) {
        printf("0x%02x 0x%02x 0x%02x 0x%02x\n",
               (unsigned int)courant->valeur.rouge,
               (unsigned int)courant->valeur.vert,
               (unsigned int)courant->valeur.bleu,
               (unsigned int)courant->valeur.alpha);
    }
}

void detruire_liste(struct liste_couleurs *liste)
{
    struct noeud_couleur *courant;

    if (liste == NULL) {
        return;
    }
    courant = liste->tete;
    while (courant != NULL) {
        struct noeud_couleur *suivant = courant->suivant;
        free(courant);
        courant = suivant;
    }
    liste->tete = NULL;
}
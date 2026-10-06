#ifndef SERVEUR_H
#define SERVEUR_H

#include <stddef.h>

#include "couleur.h"

#define PORT 8089
#define TAILLE_REQUETE 16384

extern const char *svg_file_path;

int recois_envoie_message(int socket_client_fd, char *data,
                          size_t capacite, int ouvrir_navigateur);
int plot(const couleur_compteur *couleurs);

#endif

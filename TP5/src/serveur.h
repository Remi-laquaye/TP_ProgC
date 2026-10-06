/*
 * SPDX-FileCopyrightText: 2021 John Samuel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef SERVEUR_H
#define SERVEUR_H

#include <stddef.h>

#define PORT 8089

int renvoie_message(int socket_client, const char *message);
int recois_numeros_calcule(const char *requete, char *reponse,
                           size_t capacite);

#endif

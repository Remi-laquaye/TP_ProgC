/*
 * SPDX-FileCopyrightText: 2021 John Samuel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef CLIENT_H
#define CLIENT_H

#define PORT 8089

int envoie_recois_message(int socketfd);
int envoie_operateur_numeros(int socketfd, char operateur,
                             long long num1, long long num2);

#endif

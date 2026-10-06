/*
 * SPDX-FileCopyrightText: 2021 John Samuel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef BMP_H
#define BMP_H

#include <stddef.h>
#include <stdint.h>
#include "couleur.h"

typedef struct
{
  uint16_t type;
  uint32_t file_size;
  uint16_t reserved1;
  uint16_t reserved2;
  uint32_t offset;
} bmp_header;

typedef struct
{
  uint32_t info_header_size;
  uint32_t largeur;
  uint32_t hauteur;
  uint16_t planes;
  uint16_t compte_bit;
  uint32_t compression;
  uint32_t taille_image;
  uint32_t xpixels_par_metre;
  uint32_t ypixels_par_metre;
  uint32_t couleurs_utilise;
  uint32_t couleurs_important;
} bmp_info_header;

couleur_compteur *analyse_bmp_image(const char *nom_de_fichier,
                                    size_t nombre_couleurs);

#endif

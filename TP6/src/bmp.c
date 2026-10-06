#define _POSIX_C_SOURCE 200809L

#include "bmp.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static uint16_t lire_u16(const unsigned char *donnees)
{
    return (uint16_t)donnees[0] | (uint16_t)((uint16_t)donnees[1] << 8);
}

static uint32_t lire_u32(const unsigned char *donnees)
{
    return (uint32_t)donnees[0] | ((uint32_t)donnees[1] << 8) |
           ((uint32_t)donnees[2] << 16) | ((uint32_t)donnees[3] << 24);
}

static int lire_exactement(FILE *fichier, void *tampon, size_t taille)
{
    if (fread(tampon, 1, taille, fichier) != taille) {
        if (ferror(fichier)) {
            perror("Lecture BMP");
        } else {
            fprintf(stderr, "Fichier BMP tronque.\n");
        }
        return 0;
    }
    return 1;
}

static void liberer_image(couleur *image)
{
    if (image->compte_bit == BITS24) {
        free(image->c.c24);
    } else if (image->compte_bit == BITS32) {
        free(image->c.c32);
    }
}

couleur_compteur *analyse_bmp_image(const char *nom_de_fichier,
                                    size_t nombre_couleurs)
{
    unsigned char entete[54];
    FILE *fichier;
    struct stat informations_fichier;
    uint32_t offset_pixels;
    uint32_t taille_entete;
    uint32_t compression;
    uint32_t largeur;
    uint32_t hauteur_brute;
    uint16_t bits_par_pixel;
    int hauteur_negative;
    size_t hauteur;
    size_t octets_pixel;
    size_t octets_ligne;
    size_t nombre_pixels;
    size_t taille_pixels;
    unsigned char *ligne_pixels;
    couleur image = {0};
    couleur_compteur *compteurs;

    if (nom_de_fichier == NULL || nombre_couleurs == 0 ||
        nombre_couleurs > MAX_COULEURS) {
        fprintf(stderr, "Nombre de couleurs invalide (1 a %d).\n",
                MAX_COULEURS);
        return NULL;
    }
    fichier = fopen(nom_de_fichier, "rb");
    if (fichier == NULL) {
        perror(nom_de_fichier);
        return NULL;
    }
    if (fstat(fileno(fichier), &informations_fichier) < 0) {
        perror("Informations du fichier BMP");
        fclose(fichier);
        return NULL;
    }
    if (!lire_exactement(fichier, entete, sizeof entete)) {
        fclose(fichier);
        return NULL;
    }
    if (lire_u16(entete) != UINT16_C(0x4d42)) {
        fprintf(stderr, "%s n'est pas un fichier BMP.\n", nom_de_fichier);
        fclose(fichier);
        return NULL;
    }

    offset_pixels = lire_u32(entete + 10);
    taille_entete = lire_u32(entete + 14);
    largeur = lire_u32(entete + 18);
    hauteur_brute = lire_u32(entete + 22);
    bits_par_pixel = lire_u16(entete + 28);
    compression = lire_u32(entete + 30);
    hauteur_negative = (hauteur_brute & UINT32_C(0x80000000)) != 0;
    hauteur = hauteur_negative
                  ? (size_t)(-(int64_t)(int32_t)hauteur_brute)
                  : (size_t)hauteur_brute;

    if (taille_entete < 40 ||
        (uint64_t)offset_pixels < UINT64_C(14) + taille_entete ||
        largeur == 0 || hauteur == 0 || largeur > INT_MAX ||
        hauteur > INT_MAX || (bits_par_pixel != 24 && bits_par_pixel != 32) ||
        compression != 0) {
        fprintf(stderr, "Format BMP non pris en charge (24/32 bits non compresse requis).\n");
        fclose(fichier);
        return NULL;
    }
    octets_pixel = bits_par_pixel / 8;
    if ((size_t)largeur > (SIZE_MAX - 3) / octets_pixel) {
        fprintf(stderr, "Dimensions BMP trop grandes.\n");
        fclose(fichier);
        return NULL;
    }
    octets_ligne = ((size_t)largeur * octets_pixel + 3) & ~(size_t)3;
    if (hauteur > SIZE_MAX / octets_ligne ||
        (size_t)largeur > SIZE_MAX / hauteur) {
        fprintf(stderr, "Dimensions BMP trop grandes.\n");
        fclose(fichier);
        return NULL;
    }
    taille_pixels = octets_ligne * hauteur;
    nombre_pixels = (size_t)largeur * hauteur;
    if (nombre_pixels > SIZE_MAX / sizeof(couleur32)) {
        fprintf(stderr, "Image BMP trop grande pour la memoire disponible.\n");
        fclose(fichier);
        return NULL;
    }
    if (offset_pixels > (uint64_t)informations_fichier.st_size ||
        taille_pixels > (uint64_t)informations_fichier.st_size - offset_pixels) {
        fprintf(stderr, "Les pixels BMP depassent la taille du fichier.\n");
        fclose(fichier);
        return NULL;
    }

    if (bits_par_pixel == 24) {
        image.compte_bit = BITS24;
        image.c.c24 = malloc(nombre_pixels * sizeof *image.c.c24);
        if (image.c.c24 == NULL) {
            perror("Allocation des pixels BMP");
            fclose(fichier);
            return NULL;
        }
    } else {
        image.compte_bit = BITS32;
        image.c.c32 = malloc(nombre_pixels * sizeof *image.c.c32);
        if (image.c.c32 == NULL) {
            perror("Allocation des pixels BMP");
            fclose(fichier);
            return NULL;
        }
    }

    if (fseeko(fichier, (off_t)offset_pixels, SEEK_SET) != 0) {
        perror("Positionnement dans le BMP");
        liberer_image(&image);
        fclose(fichier);
        return NULL;
    }
    ligne_pixels = malloc(octets_ligne);
    if (ligne_pixels == NULL) {
        perror("Allocation d'une ligne BMP");
        liberer_image(&image);
        fclose(fichier);
        return NULL;
    }
    for (size_t y = 0; y < hauteur; ++y) {
        size_t ligne_destination = hauteur_negative ? y : hauteur - 1 - y;

        if (!lire_exactement(fichier, ligne_pixels, octets_ligne)) {
            free(ligne_pixels);
            liberer_image(&image);
            fclose(fichier);
            return NULL;
        }
        for (size_t x = 0; x < largeur; ++x) {
            const unsigned char *pixel = ligne_pixels + x * octets_pixel;
            size_t index = ligne_destination * largeur + x;

            if (bits_par_pixel == 24) {
                image.c.c24[index].bleu = pixel[0];
                image.c.c24[index].vert = pixel[1];
                image.c.c24[index].rouge = pixel[2];
            } else {
                image.c.c32[index].bleu = pixel[0];
                image.c.c32[index].vert = pixel[1];
                image.c.c32[index].rouge = pixel[2];
                image.c.c32[index].alpha = pixel[3];
            }
        }
    }
    free(ligne_pixels);
    if (fclose(fichier) != 0) {
        perror("Fermeture du BMP");
        liberer_image(&image);
        return NULL;
    }

    compteurs = compte_couleur(&image, nombre_pixels);
    liberer_image(&image);
    if (compteurs == NULL) {
        return NULL;
    }
    trier_couleur_compteur(compteurs);
    if (compteurs->size > nombre_couleurs) {
        compteurs->size = nombre_couleurs;
    }
    return compteurs;
}

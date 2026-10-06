#define _POSIX_C_SOURCE 200809L

#include "bmp.h"
#include "client.h"

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define TAILLE_MESSAGE 16384

static int ajouter_json_chaine(char *destination, size_t capacite,
                               size_t *utilise, const char *texte)
{
    const unsigned char *caractere = (const unsigned char *)texte;

    if (*utilise + 1 >= capacite) {
        return 0;
    }
    destination[(*utilise)++] = '"';
    while (*caractere != '\0') {
        const char *echappement = NULL;
        char hexadecimal[7];
        switch (*caractere) {
        case '"': echappement = "\\\""; break;
        case '\\': echappement = "\\\\"; break;
        case '\b': echappement = "\\b"; break;
        case '\f': echappement = "\\f"; break;
        case '\n': echappement = "\\n"; break;
        case '\r': echappement = "\\r"; break;
        case '\t': echappement = "\\t"; break;
        default:
            if (*caractere < 0x20) {
                (void)snprintf(hexadecimal, sizeof hexadecimal,
                               "\\u%04x", *caractere);
                echappement = hexadecimal;
            }
            break;
        }
        if (echappement != NULL) {
            size_t longueur = strlen(echappement);
            if (*utilise + longueur >= capacite) {
                return 0;
            }
            memcpy(destination + *utilise, echappement, longueur);
            *utilise += longueur;
        } else {
            if (*utilise + 1 >= capacite) {
                return 0;
            }
            destination[(*utilise)++] = (char)*caractere;
        }
        ++caractere;
    }
    if (*utilise + 1 >= capacite) {
        return 0;
    }
    destination[(*utilise)++] = '"';
    destination[*utilise] = '\0';
    return 1;
}

static int envoyer_tout(int socketfd, const char *donnees, size_t longueur)
{
    size_t envoye = 0;

    while (envoye < longueur) {
        ssize_t resultat = send(socketfd, donnees + envoye,
                                longueur - envoye, 0);
        if (resultat < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("Envoi au serveur");
            return 0;
        }
        if (resultat == 0) {
            fprintf(stderr, "Connexion fermee pendant l'envoi.\n");
            return 0;
        }
        envoye += (size_t)resultat;
    }
    return 1;
}

static int envoyer_recevoir(int socketfd, const char *requete,
                            char *reponse, size_t capacite_reponse)
{
    size_t recu = 0;

    if (!envoyer_tout(socketfd, requete, strlen(requete))) {
        return 0;
    }
    while (recu + 1 < capacite_reponse) {
        char caractere;
        ssize_t resultat = recv(socketfd, &caractere, 1, 0);
        if (resultat < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("Reception du serveur");
            return 0;
        }
        if (resultat == 0) {
            fprintf(stderr, "Le serveur a ferme la connexion.\n");
            return 0;
        }
        reponse[recu++] = caractere;
        if (caractere == '\n') {
            reponse[recu] = '\0';
            printf("%s", reponse);
            if (strstr(reponse, "\"code\":\"erreur\"") != NULL) {
                return 0;
            }
            return 1;
        }
    }
    fprintf(stderr, "Reponse du serveur trop longue.\n");
    return 0;
}

static int construire_requete_couleurs(const couleur_compteur *compteurs,
                                       size_t nombre, char *requete,
                                       size_t capacite)
{
    int longueur;
    size_t utilise;

    longueur = snprintf(requete, capacite,
                        "{\"code\":\"couleurs\",\"nombre\":%zu,"
                        "\"valeurs\":[", nombre);
    if (longueur < 0 || (size_t)longueur >= capacite) {
        return 0;
    }
    utilise = (size_t)longueur;
    for (size_t i = 0; i < nombre; ++i) {
        char hex[8];
        if (compteurs->compte_bit == BITS24) {
            const couleur24 *couleur = &compteurs->cc.cc24[i].c;
            (void)snprintf(hex, sizeof hex, "#%02x%02x%02x",
                           couleur->rouge, couleur->vert, couleur->bleu);
        } else {
            const couleur32 *couleur = &compteurs->cc.cc32[i].c;
            (void)snprintf(hex, sizeof hex, "#%02x%02x%02x",
                           couleur->rouge, couleur->vert, couleur->bleu);
        }
        if (i != 0) {
            if (utilise + 1 >= capacite) {
                return 0;
            }
            requete[utilise++] = ',';
        }
        if (!ajouter_json_chaine(requete, capacite, &utilise, hex)) {
            return 0;
        }
    }
    if (utilise + 3 >= capacite) {
        return 0;
    }
    memcpy(requete + utilise, "]}\n", 4);
    return 1;
}

static int connecter_serveur(void)
{
    struct sockaddr_in adresse;
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);

    if (socketfd < 0) {
        perror("socket");
        return -1;
    }
    memset(&adresse, 0, sizeof adresse);
    adresse.sin_family = AF_INET;
    adresse.sin_port = htons(PORT);
    adresse.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (connect(socketfd, (struct sockaddr *)&adresse, sizeof adresse) < 0) {
        perror("Connexion au serveur");
        close(socketfd);
        return -1;
    }
    return socketfd;
}

static int envoyer_image(int socketfd, const char *chemin, size_t nombre)
{
    couleur_compteur *compteurs = analyse_bmp_image(chemin, nombre);
    char requete[TAILLE_MESSAGE];
    char reponse[TAILLE_MESSAGE];
    int resultat = EXIT_FAILURE;

    if (compteurs == NULL) {
        return EXIT_FAILURE;
    }
    if (!construire_requete_couleurs(compteurs, compteurs->size,
                                    requete, sizeof requete)) {
        fprintf(stderr, "Impossible de construire la requete JSON.\n");
    } else if (envoyer_recevoir(socketfd, requete,
                                reponse, sizeof reponse)) {
        resultat = EXIT_SUCCESS;
    }
    liberer_couleur_compteur(compteurs);
    return resultat;
}

static int envoyer_message(int socketfd, const char *texte)
{
    char requete[TAILLE_MESSAGE];
    char reponse[TAILLE_MESSAGE];
    size_t utilise;
    int longueur = snprintf(requete, sizeof requete,
                            "{\"code\":\"message\",\"valeurs\":[");

    if (longueur < 0 || (size_t)longueur >= sizeof requete) {
        return EXIT_FAILURE;
    }
    utilise = (size_t)longueur;
    if (!ajouter_json_chaine(requete, sizeof requete, &utilise, texte) ||
        utilise + 3 >= sizeof requete) {
        fprintf(stderr, "Message trop long.\n");
        return EXIT_FAILURE;
    }
    memcpy(requete + utilise, "]}\n", 4);
    return envoyer_recevoir(socketfd, requete, reponse, sizeof reponse)
               ? EXIT_SUCCESS : EXIT_FAILURE;
}

static int envoyer_calcul(int socketfd, const char *operateur,
                          const char *nombre1, const char *nombre2)
{
    char requete[TAILLE_MESSAGE];
    char reponse[TAILLE_MESSAGE];
    size_t utilise;
    int longueur = snprintf(requete, sizeof requete,
                            "{\"code\":\"calcule\",\"valeurs\":[");

    if (longueur < 0 || (size_t)longueur >= sizeof requete) {
        return EXIT_FAILURE;
    }
    utilise = (size_t)longueur;
    if (!ajouter_json_chaine(requete, sizeof requete, &utilise, operateur) ||
        utilise + 1 >= sizeof requete) {
        return EXIT_FAILURE;
    }
    requete[utilise++] = ',';
    if (!ajouter_json_chaine(requete, sizeof requete, &utilise, nombre1) ||
        utilise + 1 >= sizeof requete) {
        return EXIT_FAILURE;
    }
    requete[utilise++] = ',';
    if (!ajouter_json_chaine(requete, sizeof requete, &utilise, nombre2) ||
        utilise + 3 >= sizeof requete) {
        return EXIT_FAILURE;
    }
    memcpy(requete + utilise, "]}\n", 4);
    return envoyer_recevoir(socketfd, requete, reponse, sizeof reponse)
               ? EXIT_SUCCESS : EXIT_FAILURE;
}

static int lire_nombre_couleurs(const char *texte, size_t *nombre)
{
    char *fin;
    unsigned long valeur;

    errno = 0;
    valeur = strtoul(texte, &fin, 10);
    if (texte == fin || *fin != '\0' || errno == ERANGE ||
        valeur == 0 || valeur > MAX_COULEURS) {
        return 0;
    }
    *nombre = (size_t)valeur;
    return 1;
}

int main(int argc, char **argv)
{
    int socketfd;
    int resultat;

    if (argc < 2) {
        fprintf(stderr,
                "Usage : %s <image.bmp> [nombre_couleurs 1-%d]\n"
                "       %s --message <texte>\n"
                "       %s --calcul <operateur> <nombre1> <nombre2>\n",
                argv[0], MAX_COULEURS, argv[0], argv[0]);
        return EXIT_FAILURE;
    }
    socketfd = connecter_serveur();
    if (socketfd < 0) {
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "--message") == 0 && argc == 3) {
        resultat = envoyer_message(socketfd, argv[2]);
    } else if (strcmp(argv[1], "--calcul") == 0 && argc == 5 &&
               strlen(argv[2]) == 1) {
        resultat = envoyer_calcul(socketfd, argv[2], argv[3], argv[4]);
    } else if (argc == 2 || argc == 3) {
        size_t nombre = 10;
        if (argc == 3 && !lire_nombre_couleurs(argv[2], &nombre)) {
            fprintf(stderr, "Le nombre de couleurs doit etre entre 1 et %d.\n",
                    MAX_COULEURS);
            close(socketfd);
            return EXIT_FAILURE;
        }
        resultat = envoyer_image(socketfd, argv[1], nombre);
    } else {
        fprintf(stderr, "Arguments invalides.\n");
        close(socketfd);
        return EXIT_FAILURE;
    }
    if (close(socketfd) < 0) {
        perror("Fermeture de la socket");
        resultat = EXIT_FAILURE;
    }
    return resultat;
}

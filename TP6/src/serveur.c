#define _POSIX_C_SOURCE 200809L

#include "serveur.h"

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define TAILLE_CHAINE 512
#define PI 3.14159265358979323846

const char *svg_file_path = "pie_chart.svg";

typedef struct {
    char code[32];
    size_t nombre;
    int a_nombre;
    size_t nombre_valeurs;
    char valeurs[MAX_COULEURS][TAILLE_CHAINE];
} requete_json;

static int socket_serveur = -1;
static volatile sig_atomic_t arret_demande;

static void gerer_sigint(int signal_recu)
{
    (void)signal_recu;
    arret_demande = 1;
    if (socket_serveur >= 0) {
        close(socket_serveur);
        socket_serveur = -1;
    }
}

static void espaces(const char **curseur)
{
    while (isspace((unsigned char)**curseur)) {
        ++*curseur;
    }
}

static int lire_hex4(const char **curseur, uint32_t *valeur)
{
    uint32_t resultat = 0;

    for (int i = 0; i < 4; ++i) {
        unsigned char caractere = (unsigned char)*(*curseur)++;
        if (caractere == '\0') {
            return 0;
        }
        resultat <<= 4;
        if (caractere >= '0' && caractere <= '9') {
            resultat |= (uint32_t)(caractere - '0');
        } else if (caractere >= 'a' && caractere <= 'f') {
            resultat |= (uint32_t)(caractere - 'a' + 10);
        } else if (caractere >= 'A' && caractere <= 'F') {
            resultat |= (uint32_t)(caractere - 'A' + 10);
        } else {
            return 0;
        }
    }
    *valeur = resultat;
    return 1;
}

static int lire_chaine_json(const char **curseur, char *destination,
                            size_t capacite)
{
    size_t longueur = 0;

    espaces(curseur);
    if (**curseur != '"') {
        return 0;
    }
    ++*curseur;
    while (**curseur != '\0' && **curseur != '"') {
        unsigned char caractere = (unsigned char)*(*curseur)++;
        if (caractere == '\\') {
            caractere = (unsigned char)*(*curseur)++;
            switch (caractere) {
            case '"': case '\\': case '/': break;
            case 'b': caractere = '\b'; break;
            case 'f': caractere = '\f'; break;
            case 'n': caractere = '\n'; break;
            case 'r': caractere = '\r'; break;
            case 't': caractere = '\t'; break;
            case 'u': {
                uint32_t codepoint;
                unsigned char utf8[4];
                size_t taille_utf8;

                if (!lire_hex4(curseur, &codepoint)) {
                    return 0;
                }
                if (codepoint >= UINT32_C(0xd800) &&
                    codepoint <= UINT32_C(0xdbff)) {
                    uint32_t bas;
                    if ((*curseur)[0] != '\\' || (*curseur)[1] != 'u') {
                        return 0;
                    }
                    *curseur += 2;
                    if (!lire_hex4(curseur, &bas) ||
                        bas < UINT32_C(0xdc00) || bas > UINT32_C(0xdfff)) {
                        return 0;
                    }
                    codepoint = UINT32_C(0x10000) +
                                ((codepoint - UINT32_C(0xd800)) << 10) +
                                (bas - UINT32_C(0xdc00));
                } else if (codepoint >= UINT32_C(0xdc00) &&
                           codepoint <= UINT32_C(0xdfff)) {
                    return 0;
                }
                if (codepoint == 0) {
                    return 0;
                }
                if (codepoint <= 0x7f) {
                    utf8[0] = (unsigned char)codepoint;
                    taille_utf8 = 1;
                } else if (codepoint <= 0x7ff) {
                    utf8[0] = (unsigned char)(0xc0 | (codepoint >> 6));
                    utf8[1] = (unsigned char)(0x80 | (codepoint & 0x3f));
                    taille_utf8 = 2;
                } else if (codepoint <= 0xffff) {
                    utf8[0] = (unsigned char)(0xe0 | (codepoint >> 12));
                    utf8[1] = (unsigned char)(0x80 |
                                             ((codepoint >> 6) & 0x3f));
                    utf8[2] = (unsigned char)(0x80 | (codepoint & 0x3f));
                    taille_utf8 = 3;
                } else {
                    utf8[0] = (unsigned char)(0xf0 | (codepoint >> 18));
                    utf8[1] = (unsigned char)(0x80 |
                                             ((codepoint >> 12) & 0x3f));
                    utf8[2] = (unsigned char)(0x80 |
                                             ((codepoint >> 6) & 0x3f));
                    utf8[3] = (unsigned char)(0x80 | (codepoint & 0x3f));
                    taille_utf8 = 4;
                }
                if (longueur + taille_utf8 >= capacite) {
                    return 0;
                }
                memcpy(destination + longueur, utf8, taille_utf8);
                longueur += taille_utf8;
                continue;
            }
            default:
                return 0;
            }
        } else if (caractere < 0x20) {
            return 0;
        }
        if (longueur + 1 >= capacite) {
            return 0;
        }
        destination[longueur++] = (char)caractere;
    }
    if (**curseur != '"') {
        return 0;
    }
    ++*curseur;
    destination[longueur] = '\0';
    return 1;
}

static int lire_valeurs_json(const char **curseur, requete_json *requete)
{
    espaces(curseur);
    if (*(*curseur)++ != '[') {
        return 0;
    }
    espaces(curseur);
    if (**curseur == ']') {
        ++*curseur;
        return 1;
    }
    while (requete->nombre_valeurs < MAX_COULEURS) {
        if (!lire_chaine_json(curseur,
                              requete->valeurs[requete->nombre_valeurs],
                              sizeof requete->valeurs[0])) {
            return 0;
        }
        ++requete->nombre_valeurs;
        espaces(curseur);
        if (**curseur == ']') {
            ++*curseur;
            return 1;
        }
        if (*(*curseur)++ != ',') {
            return 0;
        }
    }
    return 0;
}

static int analyser_json(const char *texte, requete_json *requete)
{
    const char *curseur = texte;
    int a_code = 0;
    int a_valeurs = 0;

    memset(requete, 0, sizeof *requete);
    espaces(&curseur);
    if (*curseur++ != '{') {
        return 0;
    }
    espaces(&curseur);
    while (*curseur != '}') {
        char cle[32];

        if (!lire_chaine_json(&curseur, cle, sizeof cle)) {
            return 0;
        }
        espaces(&curseur);
        if (*curseur++ != ':') {
            return 0;
        }
        if (strcmp(cle, "code") == 0) {
            if (a_code || !lire_chaine_json(&curseur, requete->code,
                                            sizeof requete->code)) {
                return 0;
            }
            a_code = 1;
        } else if (strcmp(cle, "nombre") == 0) {
            char *fin;
            unsigned long nombre;
            espaces(&curseur);
            errno = 0;
            nombre = strtoul(curseur, &fin, 10);
            if (curseur == fin || errno == ERANGE ||
                nombre > MAX_COULEURS) {
                return 0;
            }
            requete->nombre = (size_t)nombre;
            requete->a_nombre = 1;
            curseur = fin;
        } else if (strcmp(cle, "valeurs") == 0) {
            if (a_valeurs || !lire_valeurs_json(&curseur, requete)) {
                return 0;
            }
            a_valeurs = 1;
        } else {
            return 0;
        }
        espaces(&curseur);
        if (*curseur == '}') {
            break;
        }
        if (*curseur++ != ',') {
            return 0;
        }
        espaces(&curseur);
    }
    if (*curseur++ != '}') {
        return 0;
    }
    espaces(&curseur);
    return *curseur == '\0' && a_code && a_valeurs;
}

static int envoyer_tout(int socketfd, const char *donnees, size_t longueur)
{
    size_t envoye = 0;

    while (envoye < longueur) {
        ssize_t resultat = send(socketfd, donnees + envoye,
                                longueur - envoye, MSG_NOSIGNAL);
        if (resultat < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("Envoi au client");
            return 0;
        }
        if (resultat == 0) {
            return 0;
        }
        envoye += (size_t)resultat;
    }
    return 1;
}

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

static int repondre_json(int socketfd, const char *reponse)
{
    size_t longueur = strlen(reponse);
    char *ligne = malloc(longueur + 2);
    int succes;

    if (ligne == NULL) {
        perror("Allocation de la reponse");
        return 0;
    }
    memcpy(ligne, reponse, longueur);
    ligne[longueur] = '\n';
    ligne[longueur + 1] = '\0';
    succes = envoyer_tout(socketfd, ligne, longueur + 1);
    free(ligne);
    return succes;
}

static int couleur_svg_valide(const char *couleur)
{
    if (strlen(couleur) != 7 || couleur[0] != '#') {
        return 0;
    }
    for (size_t i = 1; i < 7; ++i) {
        if (!isxdigit((unsigned char)couleur[i])) {
            return 0;
        }
    }
    return 1;
}

int plot(const couleur_compteur *couleurs)
{
    FILE *fichier;
    const double centre_x = 200.0;
    const double centre_y = 200.0;
    const double rayon = 150.0;
    double angle_depart = -90.0;

    if (couleurs == NULL || couleurs->size == 0 ||
        couleurs->size > MAX_COULEURS) {
        fprintf(stderr, "Aucune couleur valide pour le graphique.\n");
        return 0;
    }
    fichier = fopen(svg_file_path, "w");
    if (fichier == NULL) {
        perror(svg_file_path);
        return 0;
    }
    fprintf(fichier, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(fichier, "<svg xmlns=\"http://www.w3.org/2000/svg\" "
                     "width=\"400\" height=\"400\" viewBox=\"0 0 400 400\">\n");
    fprintf(fichier, "<rect width=\"400\" height=\"400\" fill=\"#fff\"/>\n");
    for (size_t i = 0; i < couleurs->size; ++i) {
        char couleur[8];
        double angle_fin = angle_depart + 360.0 / (double)couleurs->size;
        double x1;
        double y1;
        double x2;
        double y2;
        unsigned int grand_arc = (angle_fin - angle_depart) > 180.0;

        if (couleurs->compte_bit == BITS24) {
            const couleur24 *c = &couleurs->cc.cc24[i].c;
            (void)snprintf(couleur, sizeof couleur, "#%02x%02x%02x",
                           c->rouge, c->vert, c->bleu);
        } else {
            const couleur32 *c = &couleurs->cc.cc32[i].c;
            (void)snprintf(couleur, sizeof couleur, "#%02x%02x%02x",
                           c->rouge, c->vert, c->bleu);
        }
        if (!couleur_svg_valide(couleur)) {
            fclose(fichier);
            fprintf(stderr, "Couleur invalide pour le SVG.\n");
            return 0;
        }
        if (couleurs->size == 1) {
            fprintf(fichier,
                    "<circle cx=\"200\" cy=\"200\" r=\"150\" fill=\"%s\"/>\n",
                    couleur);
            break;
        }
        x1 = centre_x + rayon * cos(angle_depart * (PI / 180.0));
        y1 = centre_y + rayon * sin(angle_depart * (PI / 180.0));
        x2 = centre_x + rayon * cos(angle_fin * (PI / 180.0));
        y2 = centre_y + rayon * sin(angle_fin * (PI / 180.0));
        fprintf(fichier,
                "<path d=\"M %.3f %.3f A %.3f %.3f 0 %u 1 %.3f %.3f "
                "L %.3f %.3f Z\" fill=\"%s\"/>\n",
                x1, y1, rayon, rayon, grand_arc, x2, y2,
                centre_x, centre_y, couleur);
        angle_depart = angle_fin;
    }
    fprintf(fichier, "</svg>\n");
    if (fclose(fichier) != 0) {
        perror("Fermeture du SVG");
        return 0;
    }
    return 1;
}

static int ouvrir_svg(void)
{
    pid_t processus;

    if (getenv("DISPLAY") == NULL && getenv("WAYLAND_DISPLAY") == NULL) {
        fprintf(stderr, "Pas d'affichage graphique; SVG genere sans navigateur.\n");
        return 1;
    }
    processus = fork();
    if (processus < 0) {
        perror("Lancement de Firefox");
        return 0;
    }
    if (processus == 0) {
        execlp("firefox", "firefox", svg_file_path, (char *)NULL);
        _exit(EXIT_FAILURE);
    }
    return 1;
}

static int lire_entier(const char *texte, long long *valeur)
{
    char *fin;

    errno = 0;
    *valeur = strtoll(texte, &fin, 10);
    return texte != fin && *fin == '\0' && errno != ERANGE;
}

static int traiter_calcul(const requete_json *requete, char *reponse,
                          size_t capacite)
{
    long long a;
    long long b;
    long long resultat;
    char operateur;
    int longueur;

    if (requete->nombre_valeurs != 3 ||
        strlen(requete->valeurs[0]) != 1 ||
        !lire_entier(requete->valeurs[1], &a) ||
        !lire_entier(requete->valeurs[2], &b)) {
        return snprintf(reponse, capacite,
                        "{\"code\":\"erreur\",\"message\":"
                        "\"arguments de calcul invalides\"}") > 0;
    }
    operateur = requete->valeurs[0][0];
    switch (operateur) {
    case '+':
        if (__builtin_add_overflow(a, b, &resultat)) {
            goto depassement;
        }
        break;
    case '-':
        if (__builtin_sub_overflow(a, b, &resultat)) {
            goto depassement;
        }
        break;
    case '*':
        if (__builtin_mul_overflow(a, b, &resultat)) {
            goto depassement;
        }
        break;
    case '/':
        if (b == 0 || (a == LLONG_MIN && b == -1)) {
            return snprintf(reponse, capacite,
                            "{\"code\":\"erreur\",\"message\":"
                            "\"division impossible\"}") > 0;
        }
        longueur = snprintf(reponse, capacite,
                            "{\"code\":\"resultat\",\"valeur\":%.12g}",
                            (double)a / (double)b);
        return longueur >= 0 && (size_t)longueur < capacite;
    case '%':
        if (b == 0 || (a == LLONG_MIN && b == -1)) {
            return snprintf(reponse, capacite,
                            "{\"code\":\"erreur\",\"message\":"
                            "\"modulo impossible\"}") > 0;
        }
        resultat = a % b;
        break;
    case '&': resultat = a & b; break;
    case '|': resultat = a | b; break;
    case '~': resultat = ~a; break;
    default:
        return snprintf(reponse, capacite,
                        "{\"code\":\"erreur\",\"message\":"
                        "\"operateur invalide\"}") > 0;
    }
    longueur = snprintf(reponse, capacite,
                        "{\"code\":\"resultat\",\"valeur\":%lld}",
                        resultat);
    return longueur >= 0 && (size_t)longueur < capacite;

depassement:
    longueur = snprintf(reponse, capacite,
                        "{\"code\":\"erreur\",\"message\":"
                        "\"depassement arithmetique\"}");
    return longueur >= 0 && (size_t)longueur < capacite;
}

static int traiter_requete(const requete_json *requete, char *reponse,
                           size_t capacite, int ouvrir_navigateur)
{
    if (strcmp(requete->code, "message") == 0) {
        size_t utilise = 0;
        int longueur;

        if (requete->nombre_valeurs != 1) {
            return 0;
        }
        longueur = snprintf(reponse, capacite,
                            "{\"code\":\"message\",\"valeurs\":[");
        if (longueur < 0 || (size_t)longueur >= capacite) {
            return 0;
        }
        utilise = (size_t)longueur;
        if (!ajouter_json_chaine(reponse, capacite, &utilise,
                                 requete->valeurs[0]) ||
            utilise + 3 >= capacite) {
            return 0;
        }
        memcpy(reponse + utilise, "]}", 3);
        return 1;
    }
    if (strcmp(requete->code, "calcule") == 0) {
        return traiter_calcul(requete, reponse, capacite);
    }
    if (strcmp(requete->code, "couleurs") == 0) {
        couleur_compteur couleurs = {0};
        size_t nombre = requete->a_nombre ? requete->nombre
                                          : requete->nombre_valeurs;

        if (nombre == 0 || nombre > MAX_COULEURS ||
            requete->nombre_valeurs != nombre) {
            return snprintf(reponse, capacite,
                            "{\"code\":\"erreur\",\"message\":"
                            "\"nombre de couleurs invalide\"}") > 0;
        }
        couleurs.compte_bit = BITS24;
        couleurs.size = nombre;
        couleurs.cc.cc24 = calloc(couleurs.size, sizeof *couleurs.cc.cc24);
        if (couleurs.cc.cc24 == NULL) {
            perror("Allocation des couleurs SVG");
            return 0;
        }
        for (size_t i = 0; i < couleurs.size; ++i) {
            const char *hex = requete->valeurs[i];
            unsigned int rouge;
            unsigned int vert;
            unsigned int bleu;
            if (!couleur_svg_valide(hex) ||
                sscanf(hex + 1, "%2x%2x%2x", &rouge, &vert, &bleu) != 3) {
                free(couleurs.cc.cc24);
                return snprintf(reponse, capacite,
                                "{\"code\":\"erreur\",\"message\":"
                                "\"format de couleur invalide\"}") > 0;
            }
            couleurs.cc.cc24[i].c.rouge = (uint8_t)rouge;
            couleurs.cc.cc24[i].c.vert = (uint8_t)vert;
            couleurs.cc.cc24[i].c.bleu = (uint8_t)bleu;
        }
        if (!plot(&couleurs)) {
            free(couleurs.cc.cc24);
            return snprintf(reponse, capacite,
                            "{\"code\":\"erreur\",\"message\":"
                            "\"generation SVG impossible\"}") > 0;
        }
        free(couleurs.cc.cc24);
        if (ouvrir_navigateur) {
            (void)ouvrir_svg();
        }
        return snprintf(reponse, capacite,
                        "{\"code\":\"ok\",\"nombre\":%zu,"
                        "\"fichier\":\"%s\"}",
                        nombre, svg_file_path) > 0;
    }
    return snprintf(reponse, capacite,
                    "{\"code\":\"erreur\",\"message\":"
                    "\"code operation inconnu\"}") > 0;
}

int recois_envoie_message(int socket_client_fd, char *data,
                          size_t capacite, int ouvrir_navigateur)
{
    requete_json requete;
    char reponse[TAILLE_REQUETE];

    if (data == NULL || capacite == 0 || !analyser_json(data, &requete)) {
        return repondre_json(socket_client_fd,
                             "{\"code\":\"erreur\","
                             "\"message\":\"JSON invalide\"}")
                   ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    if (!traiter_requete(&requete, reponse, sizeof reponse,
                         ouvrir_navigateur)) {
        return repondre_json(socket_client_fd,
                             "{\"code\":\"erreur\","
                             "\"message\":\"traitement impossible\"}")
                   ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    return repondre_json(socket_client_fd, reponse)
               ? EXIT_SUCCESS : EXIT_FAILURE;
}

int main(int argc, char **argv)
{
    struct sockaddr_in adresse_serveur;
    struct sigaction action;
    int ouvrir_navigateur = 1;
    int option = 1;

    if (argc == 2 && strcmp(argv[1], "--no-browser") == 0) {
        ouvrir_navigateur = 0;
    } else if (argc != 1) {
        fprintf(stderr, "Usage : %s [--no-browser]\n", argv[0]);
        return EXIT_FAILURE;
    }
    memset(&action, 0, sizeof action);
    action.sa_handler = gerer_sigint;
    sigemptyset(&action.sa_mask);
    if (sigaction(SIGINT, &action, NULL) < 0) {
        perror("sigaction");
        return EXIT_FAILURE;
    }
    signal(SIGPIPE, SIG_IGN);

    socket_serveur = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_serveur < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }
    if (setsockopt(socket_serveur, SOL_SOCKET, SO_REUSEADDR,
                   &option, sizeof option) < 0) {
        perror("setsockopt");
        close(socket_serveur);
        return EXIT_FAILURE;
    }
    memset(&adresse_serveur, 0, sizeof adresse_serveur);
    adresse_serveur.sin_family = AF_INET;
    adresse_serveur.sin_port = htons(PORT);
    adresse_serveur.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(socket_serveur, (struct sockaddr *)&adresse_serveur,
             sizeof adresse_serveur) < 0) {
        perror("bind");
        close(socket_serveur);
        return EXIT_FAILURE;
    }
    if (listen(socket_serveur, 10) < 0) {
        perror("listen");
        close(socket_serveur);
        return EXIT_FAILURE;
    }
    printf("Serveur en attente de connexions sur le port %d...\n", PORT);

    while (!arret_demande) {
        struct sockaddr_in adresse_client;
        socklen_t taille_adresse = sizeof adresse_client;
        int client = accept(socket_serveur,
                            (struct sockaddr *)&adresse_client,
                            &taille_adresse);
        char data[TAILLE_REQUETE];
        size_t longueur = 0;
        int ligne_complete = 0;
        int octets_valides = 1;

        if (client < 0) {
            if (errno == EINTR || arret_demande) {
                continue;
            }
            perror("accept");
            continue;
        }
        while (longueur + 1 < sizeof data) {
            char caractere;
            ssize_t recu = recv(client, &caractere, 1, 0);
            if (recu < 0 && errno == EINTR) {
                continue;
            }
            if (recu <= 0) {
                break;
            }
            if (caractere == '\n') {
                ligne_complete = 1;
                break;
            }
            if (caractere == '\0') {
                octets_valides = 0;
                break;
            }
            data[longueur++] = caractere;
        }
        data[longueur] = '\0';
        if (longueur + 1 == sizeof data) {
            (void)repondre_json(client,
                                "{\"code\":\"erreur\","
                                "\"message\":\"requete trop longue\"}");
        } else if (!octets_valides || (longueur != 0 && !ligne_complete)) {
            (void)repondre_json(client,
                                "{\"code\":\"erreur\","
                                "\"message\":\"requete JSON incomplete\"}");
        } else if (longueur != 0) {
            (void)recois_envoie_message(client, data, sizeof data,
                                        ouvrir_navigateur);
        }
        close(client);
    }
    if (socket_serveur >= 0) {
        close(socket_serveur);
    }
    return EXIT_SUCCESS;
}

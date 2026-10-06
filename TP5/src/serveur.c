#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <ctype.h>
#include <limits.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "serveur.h"

#define TAILLE_MESSAGE 1024

static int socketfd = -1;
static volatile sig_atomic_t arret_demande = 0;

static void gestionnaire_ctrl_c(int signal_recu)
{
    static const char message[] =
        "\nSignal Ctrl+C capture. Sortie du programme.\n";
    (void)signal_recu;
    arret_demande = 1;
    if (socketfd >= 0) {
        close(socketfd);
        socketfd = -1;
    }
    (void)write(STDOUT_FILENO, message, sizeof message - 1);
}

int renvoie_message(int socket_client, const char *message)
{
    size_t longueur = strlen(message);
    size_t envoyes = 0;

    while (envoyes < longueur) {
        ssize_t resultat = send(socket_client, message + envoyes,
                                longueur - envoyes, 0);
        if (resultat < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("Erreur d'envoi");
            return EXIT_FAILURE;
        }
        if (resultat == 0) {
            fprintf(stderr, "Connexion fermee pendant l'envoi.\n");
            return EXIT_FAILURE;
        }
        envoyes += (size_t)resultat;
    }
    return EXIT_SUCCESS;
}

static int extraire_nombre(const char **curseur, long long *nombre)
{
    char *fin;

    while (isspace((unsigned char)**curseur)) {
        ++*curseur;
    }
    errno = 0;
    *nombre = strtoll(*curseur, &fin, 10);
    if (*curseur == fin || errno == ERANGE) {
        return 0;
    }
    *curseur = fin;
    return 1;
}

int recois_numeros_calcule(const char *requete, char *reponse,
                           size_t capacite)
{
    const char *curseur;
    char operateur;
    long long num1;
    long long num2;
    long long resultat;
    int longueur;

    if (capacite == 0 || strncmp(requete, "calcule :", 9) != 0) {
        return EXIT_FAILURE;
    }
    curseur = requete + 9;
    while (isspace((unsigned char)*curseur)) {
        ++curseur;
    }
    if (*curseur == '\0') {
        goto requete_invalide;
    }
    operateur = *curseur++;
    if (!isspace((unsigned char)*curseur)) {
        goto requete_invalide;
    }
    if (!extraire_nombre(&curseur, &num1) ||
        !isspace((unsigned char)*curseur) ||
        !extraire_nombre(&curseur, &num2)) {
        goto requete_invalide;
    }
    while (isspace((unsigned char)*curseur)) {
        ++curseur;
    }
    if (*curseur != '\0') {
        goto requete_invalide;
    }

    switch (operateur) {
    case '+':
        if (__builtin_add_overflow(num1, num2, &resultat)) {
            goto depassement;
        }
        break;
    case '-':
        if (__builtin_sub_overflow(num1, num2, &resultat)) {
            goto depassement;
        }
        break;
    case '*':
        if (__builtin_mul_overflow(num1, num2, &resultat)) {
            goto depassement;
        }
        break;
    case '/':
        if (num2 == 0 || (num1 == LLONG_MIN && num2 == -1)) {
            longueur = snprintf(reponse, capacite,
                                "Erreur: division impossible\n");
            return longueur >= 0 && (size_t)longueur < capacite
                       ? EXIT_SUCCESS : EXIT_FAILURE;
        }
        longueur = snprintf(reponse, capacite, "calcule : %.10g\n",
                            (double)num1 / (double)num2);
        return longueur >= 0 && (size_t)longueur < capacite
                   ? EXIT_SUCCESS : EXIT_FAILURE;
    case '%':
        if (num2 == 0 || (num1 == LLONG_MIN && num2 == -1)) {
            longueur = snprintf(reponse, capacite,
                                "Erreur: modulo impossible\n");
            return longueur >= 0 && (size_t)longueur < capacite
                       ? EXIT_SUCCESS : EXIT_FAILURE;
        }
        resultat = num1 % num2;
        break;
    case '&':
        resultat = num1 & num2;
        break;
    case '|':
        resultat = num1 | num2;
        break;
    case '~':
        resultat = ~num1;
        break;
    default:
        longueur = snprintf(reponse, capacite,
                            "Erreur: operateur non pris en charge\n");
        return longueur >= 0 && (size_t)longueur < capacite
                   ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    longueur = snprintf(reponse, capacite, "calcule : %lld\n", resultat);
    return longueur >= 0 && (size_t)longueur < capacite
               ? EXIT_SUCCESS : EXIT_FAILURE;

depassement:
    longueur = snprintf(reponse, capacite,
                        "Erreur: depassement arithmetique\n");
    return longueur >= 0 && (size_t)longueur < capacite
               ? EXIT_SUCCESS : EXIT_FAILURE;

requete_invalide:
    longueur = snprintf(reponse, capacite,
                        "Erreur: requete de calcul invalide\n");
    return longueur >= 0 && (size_t)longueur < capacite
               ? EXIT_SUCCESS : EXIT_FAILURE;
}

static int repondre_message(int client_socket_fd, const char *requete)
{
    char reponse[TAILLE_MESSAGE];
    char saisie[TAILLE_MESSAGE];
    size_t longueur_prefixe = strlen("message: ");
    size_t longueur;

    if (strncmp(requete, "message: ", longueur_prefixe) != 0) {
        return renvoie_message(client_socket_fd,
                               "Erreur: format de message invalide\n");
    }
    printf("Message recu: %s\n", requete + longueur_prefixe);
    printf("Reponse a envoyer au client : ");
    fflush(stdout);
    if (fgets(saisie, sizeof saisie, stdin) == NULL) {
        return renvoie_message(client_socket_fd,
                               "Erreur: aucune reponse saisie\n");
    }
    longueur = strlen(saisie);
    if (longueur > 0 && saisie[longueur - 1] == '\n') {
        saisie[--longueur] = '\0';
    }
    if (snprintf(reponse, sizeof reponse, "message: %s\n", saisie) >=
        (int)sizeof reponse) {
        return renvoie_message(client_socket_fd,
                               "Erreur: reponse trop longue\n");
    }
    return renvoie_message(client_socket_fd, reponse);
}

static int traiter_requete(int client_socket_fd, char *requete)
{
    char reponse[TAILLE_MESSAGE];
    int resultat;

    if (strncmp(requete, "calcule :", 9) == 0) {
        resultat = recois_numeros_calcule(requete, reponse, sizeof reponse);
        if (resultat != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }
        return renvoie_message(client_socket_fd, reponse);
    }
    return repondre_message(client_socket_fd, requete);
}

static int gerer_client(int client_socket_fd)
{
    char requete[TAILLE_MESSAGE];
    size_t longueur = 0;

    while (1) {
        char caractere;
        ssize_t resultat = recv(client_socket_fd, &caractere, 1, 0);

        if (resultat < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("Erreur de reception");
            return EXIT_FAILURE;
        }
        if (resultat == 0) {
            if (longueur != 0) {
                fprintf(stderr, "Requete incomplete recue.\n");
                return EXIT_FAILURE;
            }
            return EXIT_SUCCESS;
        }
        if (caractere == '\n') {
            requete[longueur] = '\0';
            if (traiter_requete(client_socket_fd, requete) != EXIT_SUCCESS) {
                return EXIT_FAILURE;
            }
            longueur = 0;
        } else if (longueur + 1 < sizeof requete) {
            requete[longueur++] = caractere;
        } else {
            fprintf(stderr, "Requete trop longue.\n");
            return EXIT_FAILURE;
        }
    }
}

int main(void)
{
    struct sockaddr_in adresse_serveur;
    struct sigaction action;
    int option = 1;

    (void)setvbuf(stdout, NULL, _IONBF, 0);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);
    memset(&action, 0, sizeof action);
    action.sa_handler = gestionnaire_ctrl_c;
    sigemptyset(&action.sa_mask);
    if (sigaction(SIGINT, &action, NULL) < 0) {
        perror("sigaction");
        return EXIT_FAILURE;
    }

    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &option,
                   sizeof option) < 0) {
        perror("setsockopt");
        close(socketfd);
        return EXIT_FAILURE;
    }
    memset(&adresse_serveur, 0, sizeof adresse_serveur);
    adresse_serveur.sin_family = AF_INET;
    adresse_serveur.sin_port = htons(PORT);
    adresse_serveur.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(socketfd, (struct sockaddr *)&adresse_serveur,
             sizeof adresse_serveur) < 0) {
        perror("bind");
        close(socketfd);
        return EXIT_FAILURE;
    }
    if (listen(socketfd, 10) < 0) {
        perror("listen");
        close(socketfd);
        return EXIT_FAILURE;
    }
    printf("Serveur en attente de connexions...\n");

    while (!arret_demande) {
        struct sockaddr_in adresse_client;
        socklen_t longueur_adresse = sizeof adresse_client;
        int client_socket_fd = accept(socketfd,
                                      (struct sockaddr *)&adresse_client,
                                      &longueur_adresse);
        pid_t processus;

        if (client_socket_fd < 0) {
            if (errno == EINTR && arret_demande) {
                break;
            }
            if (errno == EINTR) {
                continue;
            }
            if (arret_demande) {
                break;
            }
            perror("accept");
            continue;
        }
        processus = fork();
        if (processus == 0) {
            int statut;
            close(socketfd);
            statut = gerer_client(client_socket_fd);
            close(client_socket_fd);
            _exit(statut == EXIT_SUCCESS ? EXIT_SUCCESS : EXIT_FAILURE);
        }
        if (processus < 0) {
            perror("fork");
        }
        close(client_socket_fd);
    }

    if (socketfd >= 0) {
        close(socketfd);
    }
    return EXIT_SUCCESS;
}

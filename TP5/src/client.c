#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <ctype.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "client.h"

#define TAILLE_MESSAGE 1024

static int envoyer_tout(int socketfd, const char *donnees, size_t longueur)
{
    size_t envoyes = 0;

    while (envoyes < longueur) {
        ssize_t resultat = send(socketfd, donnees + envoyes,
                                longueur - envoyes, 0);
        if (resultat < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("Erreur d'envoi");
            return -1;
        }
        if (resultat == 0) {
            fprintf(stderr, "Connexion fermee pendant l'envoi.\n");
            return -1;
        }
        envoyes += (size_t)resultat;
    }
    return 0;
}

static int envoyer_requete(int socketfd, const char *requete,
                           char *reponse, size_t capacite_reponse)
{
    size_t longueur = strlen(requete);
    size_t recues = 0;
    int trop_longue = 0;

    if (capacite_reponse == 0) {
        return -1;
    }
    if (longueur == 0 || requete[longueur - 1] != '\n') {
        fprintf(stderr, "La requete doit se terminer par une nouvelle ligne.\n");
        return -1;
    }
    if (envoyer_tout(socketfd, requete, longueur) < 0) {
        return -1;
    }

    while (recues + 1 < capacite_reponse) {
        char caractere;
        ssize_t resultat = recv(socketfd, &caractere, 1, 0);
        if (resultat < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("Erreur de reception");
            return -1;
        }
        if (resultat == 0) {
            fprintf(stderr, "Le serveur a ferme la connexion.\n");
            return -1;
        }
        reponse[recues++] = caractere;
        if (caractere == '\n') {
            reponse[recues] = '\0';
            printf("Message recu: %s", reponse);
            return 0;
        }
    }
    while (1) {
        char caractere;
        ssize_t resultat = recv(socketfd, &caractere, 1, 0);
        if (resultat <= 0 || caractere == '\n') {
            break;
        }
        trop_longue = 1;
    }
    if (trop_longue) {
        fprintf(stderr, "Reponse du serveur trop longue.\n");
    }
    return -1;
}

static int envoyer_operation(int socketfd, char operateur, long long num1,
                             long long num2, char *reponse,
                             size_t capacite_reponse)
{
    char requete[TAILLE_MESSAGE];
    int longueur = snprintf(requete, sizeof requete,
                            "calcule : %c %lld %lld\n",
                            operateur, num1, num2);

    if (longueur < 0 || (size_t)longueur >= sizeof requete) {
        fprintf(stderr, "Operation trop longue.\n");
        return -1;
    }
    return envoyer_requete(socketfd, requete, reponse, capacite_reponse);
}

int envoie_operateur_numeros(int socketfd, char operateur,
                             long long num1, long long num2)
{
    char reponse[TAILLE_MESSAGE];

    return envoyer_operation(socketfd, operateur, num1, num2,
                             reponse, sizeof reponse);
}

static int traiter_commande_calcul(int socketfd, const char *message)
{
    const char *curseur = message + strlen("calcule :");
    char operateur;
    long long num1;
    long long num2;
    char *fin;

    while (isspace((unsigned char)*curseur)) {
        ++curseur;
    }
    if (*curseur == '\0') {
        goto format_invalide;
    }
    operateur = *curseur++;
    if (!isspace((unsigned char)*curseur)) {
        goto format_invalide;
    }
    while (isspace((unsigned char)*curseur)) {
        ++curseur;
    }
    errno = 0;
    num1 = strtoll(curseur, &fin, 10);
    if (curseur == fin || errno == ERANGE ||
        !isspace((unsigned char)*fin)) {
        goto format_invalide;
    }
    curseur = fin;
    while (isspace((unsigned char)*curseur)) {
        ++curseur;
    }
    errno = 0;
    num2 = strtoll(curseur, &fin, 10);
    if (curseur == fin || errno == ERANGE) {
        goto format_invalide;
    }
    curseur = fin;
    while (isspace((unsigned char)*curseur)) {
        ++curseur;
    }
    if (*curseur != '\0') {
        goto format_invalide;
    }
    return envoie_operateur_numeros(socketfd, operateur, num1, num2);

format_invalide:
    fprintf(stderr, "Format attendu : calcule : <operateur> <n1> <n2>\n");
    return -1;
}

int envoie_recois_message(int socketfd)
{
    char message[TAILLE_MESSAGE];
    char requete[TAILLE_MESSAGE + 16];
    size_t longueur;
    int ecrit;

    printf("Votre message ou calcul (q pour quitter): ");
    if (fgets(message, sizeof message, stdin) == NULL) {
        return 1;
    }
    longueur = strlen(message);
    if (longueur > 0 && message[longueur - 1] == '\n') {
        message[--longueur] = '\0';
    } else if (!feof(stdin)) {
        int caractere;
        while ((caractere = getchar()) != '\n' && caractere != EOF) {
        }
        fprintf(stderr, "Saisie trop longue.\n");
        return -1;
    }
    if (strcmp(message, "q") == 0 || strcmp(message, "quitter") == 0) {
        return 1;
    }
    if (strncmp(message, "calcule :", 9) == 0) {
        return traiter_commande_calcul(socketfd, message);
    }
    ecrit = snprintf(requete, sizeof requete, "message: %s\n", message);
    if (ecrit < 0 || (size_t)ecrit >= sizeof requete) {
        fprintf(stderr, "Message trop long.\n");
        return -1;
    }
    return envoyer_requete(socketfd, requete, message, sizeof message);
}

static int extraire_entier_reponse(const char *reponse, long long *valeur)
{
    const char *debut = "calcule : ";
    char *fin;

    if (strncmp(reponse, debut, strlen(debut)) != 0) {
        fprintf(stderr, "Le serveur n'a pas renvoye un resultat valide.\n");
        return -1;
    }
    errno = 0;
    *valeur = strtoll(reponse + strlen(debut), &fin, 10);
    if (fin == reponse + strlen(debut) || errno == ERANGE ||
        fin[0] != '\n' || fin[1] != '\0') {
        fprintf(stderr, "Resultat entier invalide recu du serveur.\n");
        return -1;
    }
    return 0;
}

static int extraire_reel_reponse(const char *reponse, double *valeur)
{
    const char *debut = "calcule : ";
    char *fin;

    if (strncmp(reponse, debut, strlen(debut)) != 0) {
        fprintf(stderr, "Le serveur n'a pas renvoye un resultat valide.\n");
        return -1;
    }
    errno = 0;
    *valeur = strtod(reponse + strlen(debut), &fin);
    if (fin == reponse + strlen(debut) || errno == ERANGE ||
        fin[0] != '\n' || fin[1] != '\0') {
        fprintf(stderr, "Resultat numerique invalide recu du serveur.\n");
        return -1;
    }
    return 0;
}

static int lire_note(const char *repertoire, int etudiant, int note,
                     long long *valeur)
{
    char chemin[1024];
    char contenu[128];
    char *fin;
    FILE *fichier;
    int longueur = snprintf(chemin, sizeof chemin, "%s/%d/note%d.txt",
                            repertoire, etudiant, note);

    if (longueur < 0 || (size_t)longueur >= sizeof chemin) {
        fprintf(stderr, "Chemin de note trop long.\n");
        return -1;
    }
    fichier = fopen(chemin, "r");
    if (fichier == NULL) {
        perror(chemin);
        return -1;
    }
    if (fgets(contenu, sizeof contenu, fichier) == NULL) {
        fprintf(stderr, "Impossible de lire %s.\n", chemin);
        fclose(fichier);
        return -1;
    }
    if (fclose(fichier) == EOF) {
        perror(chemin);
        return -1;
    }
    errno = 0;
    *valeur = strtoll(contenu, &fin, 10);
    while (*fin == ' ' || *fin == '\t' || *fin == '\r' || *fin == '\n') {
        ++fin;
    }
    if (contenu == fin || *fin != '\0' || errno == ERANGE) {
        fprintf(stderr, "Note invalide dans %s.\n", chemin);
        return -1;
    }
    return 0;
}

static int executer_calcul_notes(int socketfd, const char *repertoire)
{
    long long somme_classe = 0;

    for (int etudiant = 1; etudiant <= 5; ++etudiant) {
        long long somme_etudiant = 0;

        for (int note = 1; note <= 5; ++note) {
            long long valeur;

            if (lire_note(repertoire, etudiant, note, &valeur) < 0) {
                return -1;
            }
            if (note == 1) {
                somme_etudiant = valeur;
            } else {
                char reponse[TAILLE_MESSAGE];
                long long nouvelle_somme;
                if (envoyer_operation(socketfd, '+', somme_etudiant, valeur,
                                      reponse, sizeof reponse) < 0 ||
                    extraire_entier_reponse(reponse, &nouvelle_somme) < 0) {
                    return -1;
                }
                somme_etudiant = nouvelle_somme;
            }
        }
        printf("Somme des notes de l'etudiant %d : %lld\n",
               etudiant, somme_etudiant);
        if (etudiant > 1) {
            char reponse[TAILLE_MESSAGE];
            long long nouvelle_somme;
            if (envoyer_operation(socketfd, '+', somme_classe, somme_etudiant,
                                  reponse, sizeof reponse) < 0 ||
                extraire_entier_reponse(reponse, &nouvelle_somme) < 0) {
                return -1;
            }
            somme_classe = nouvelle_somme;
        } else {
            somme_classe = somme_etudiant;
        }
    }
    {
        char reponse[TAILLE_MESSAGE];
        double moyenne;
        if (envoyer_operation(socketfd, '/', somme_classe, 25,
                              reponse, sizeof reponse) < 0 ||
            extraire_reel_reponse(reponse, &moyenne) < 0) {
            return -1;
        }
        printf("Moyenne de la classe : %.2f\n", moyenne);
    }
    return 0;
}

static int connecter_serveur(void)
{
    struct sockaddr_in adresse_serveur;
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);

    if (socketfd < 0) {
        perror("socket");
        return -1;
    }
    memset(&adresse_serveur, 0, sizeof adresse_serveur);
    adresse_serveur.sin_family = AF_INET;
    adresse_serveur.sin_port = htons(PORT);
    adresse_serveur.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (connect(socketfd, (struct sockaddr *)&adresse_serveur,
                sizeof adresse_serveur) < 0) {
        perror("Connexion au serveur");
        close(socketfd);
        return -1;
    }
    return socketfd;
}

int main(int argc, char **argv)
{
    int socketfd;
    int resultat = EXIT_SUCCESS;

    if (argc > 3 || (argc >= 2 && strcmp(argv[1], "--notes") != 0)) {
        fprintf(stderr, "Usage : %s [--notes [repertoire_etudiants]]\n",
                argv[0]);
        return EXIT_FAILURE;
    }
    socketfd = connecter_serveur();
    if (socketfd < 0) {
        return EXIT_FAILURE;
    }

    if (argc >= 2) {
        const char *repertoire = argc == 3 ? argv[2] : "TP5/etudiant";
        if (executer_calcul_notes(socketfd, repertoire) < 0) {
            resultat = EXIT_FAILURE;
        }
    } else {
        while (1) {
            int statut = envoie_recois_message(socketfd);
            if (statut == 1) {
                break;
            }
            if (statut < 0) {
                resultat = EXIT_FAILURE;
                break;
            }
        }
    }
    if (close(socketfd) < 0) {
        perror("Fermeture de la socket");
        resultat = EXIT_FAILURE;
    }
    return resultat;
}

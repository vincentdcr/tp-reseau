#include <netinet/in.h>

//Type message et une liste chainee de messages

typedef struct {
    char* contenu;
    int taille_contenu;
    long date;
    char* auteur;
} message_s, *message;

typedef struct liste_message_s {
    message me;
    struct liste_message_s* prochain;
} liste_message_s, *liste_message;

//Type client et une liste chainee de clients

typedef struct {
    char* pseudo; /* 6 max */
    long derniereDeconnexion;
    struct sockaddr_in addr; // IP ?
    struct liste_client_s* abonnements;
    struct liste_client_s* abonnes;
} client_s, *client;

typedef struct liste_client_s {
    client cl;
    struct liste_client_s* prochain;
} liste_client_s, *liste_client; 


liste_message creer_liste_messages ();

message newMessage(char* content, int length, long date, char* author) ;

void insertListeMsg (liste_message listeMsg, message msg ) ;


void writeNewMsg(liste_message listeMsg, int idSocket, char* auteur, long date ) ;

liste_client creer_liste_client ();

void newClient(struct sockaddr_in adresse, char* pseudo, client client) ;

liste_client insertListeClient (liste_client listeClient, client c ) ;

client findClient(liste_client listeClient, char* pseudo );

client findClientfromAddr(liste_client listeClient, struct sockaddr_in adresse ) ;

liste_client findSubscribers(liste_client listeClient, char* pseudo ) ;

liste_client findSubscriptions(liste_client listeClient, char* pseudo ) ;

char** getNamesFromListeClient(liste_client liste) ;
#include <netinet/in.h>
#include <stdbool.h>

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
    long derniersMsgLus;
    struct sockaddr_in addr; // IP ?
    struct liste_client_s* abonnements;
    struct liste_client_s* abonnes;
} client_s, *client;

typedef struct liste_client_s {
    client cl;
    struct liste_client_s* prochain;
} liste_client_s, *liste_client; 

typedef struct connected_clients_s {
    client cl;
    int socket;
    struct connected_clients_s* prochain;
} connected_clients_s, *connected_clients;


liste_message creer_liste_messages ();


void insertListeMsg (liste_message* listeMsg, message msg ) ;


char* writeNewMsg(liste_message* listeMsg, int idSocket, char* auteur, long date ) ;

liste_client creer_liste_client ();

void insertListeClient (liste_client* listeClient, client c ) ;

client findClient(liste_client listeClient, char* pseudo );

client findClientfromAddr(liste_client listeClient, struct sockaddr_in adresse ) ;

liste_client findSubscribers(liste_client listeClient, char* pseudo ) ;

liste_client findSubscriptions(liste_client listeClient, char* pseudo ) ;

char* getNamesFromListeClient(liste_client liste) ;

client findConnectedClient(connected_clients liste, int socket);

void addSubscription (client* c_follower, client* c_following);

void rmClient(liste_client *listeptr, client c) ;

void removeSubscription (client c_follower, client c_following);

void insertConnectedClients (connected_clients* liste, client c, int socket );

void rmConnectedClient(connected_clients *listeptr, int socket);
#include <stdlib.h>
#include <stdio.h>
#include "types.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <sys/signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include  <sys/time.h>

liste_message creer_liste_messages ()
{
    return malloc(sizeof(liste_message_s));
}

message newMessage(char* content, int length, long date, char* author) {
    message m= malloc(sizeof(message));
    m->contenu = content;
    m->taille_contenu = length;
    m->date = date;
    m->auteur = author;
    return m;
}

void insertListeMsg (liste_message listeMsg, message msg ) {
    liste_message newhead = malloc(sizeof(liste_message));
    newhead->me = msg;
    newhead->prochain = listeMsg;
    listeMsg = newhead;
    return;
}


void writeNewMsg(liste_message listeMsg, int idSocket, char* auteur, long date ) {
   message m = listeMsg->me;
   //start from the beginning
   while(listeMsg != NULL && (m->date > date)) {
    if (m->auteur == auteur) {
        char msg[6+m->taille_contenu];
        sprintf(msg, "<%s> %s" , m->auteur , m->contenu );
        write(idSocket, msg, 6+m->taille_contenu );
    }
    listeMsg = listeMsg->prochain;
    if (listeMsg !=NULL )
        m = listeMsg->me;
   }
}

liste_client creer_liste_client ()
{
    return malloc(sizeof(liste_client_s));
} 

client newClient(int port, char* pseudo) {
    client c= malloc(sizeof(client));
    c->abonnements = NULL;
    c->abonnes = NULL;
    c->derniereDeconnexion = 0;
    c->port = port ;
    c->pseudo = pseudo;
    return c;
}

void insertListeClient (liste_client listeClient, client c ) {
    liste_client newhead = malloc(sizeof(liste_client));
    newhead->cl = c;
    newhead->prochain = listeClient;
    listeClient = newhead;
    return;
}

client findClient(liste_client listeClient, char* pseudo ) {
   //start from the beginning
   while(listeClient != NULL) {
    client personne = listeClient->cl;
    if (personne->pseudo == pseudo) {
        return personne;
    }
    listeClient = listeClient->prochain;
   }
   return NULL;
}

liste_client findSubscribers(liste_client listeClient, char* pseudo ) {
   //start from the beginning
   while(listeClient != NULL) {
    client personne = listeClient->cl;
    if (personne->pseudo == pseudo) {
        return personne->abonnes;
    }
    listeClient = listeClient->prochain;
   }
   return NULL;
}

liste_client findSubscriptions(liste_client listeClient, char* pseudo ) {
   //start from the beginning
   while(listeClient != NULL) {
    client personne = listeClient->cl;
    if (personne->pseudo == pseudo) {
        return personne->abonnements;
    }
    listeClient = listeClient->prochain;
   }
   return NULL;
}

char** getNamesFromListeClient(liste_client liste) {
    char** liste_pseudos; //not flexible...
    int i=0;
    while(liste != NULL) {
        client name = liste->cl;
        liste_pseudos[i]= name->pseudo;
        i++;
    }
    liste = liste->prochain;
   return liste_pseudos;
} 

//tester si les deux existent AVANT
void addSubscription (client c_follower, client c_following) {
    insertListeClient( c_follower->abonnements, c_following);
    insertListeClient( c_following->abonnements, c_follower);
}

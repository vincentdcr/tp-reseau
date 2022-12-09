#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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

void insertListeMsg (liste_message* listeMsg, message msg ) {
    liste_message newhead = malloc(sizeof(liste_message));
    newhead->me = msg;
    newhead->prochain = *listeMsg;
    *listeMsg = newhead; 
}


void writeNewMsg(liste_message* listeMsg, int idSocket, char* auteur, long date ) {
   message m = (*listeMsg)->me;
   liste_message listeparcours = *listeMsg;
   printf("writeNewMsg: %s, %ld\n", m->contenu, m->date);
   //start from the beginning
   while(listeparcours != NULL && (m->date > date)) {
    if (m->auteur == auteur) {
        char msg[6+m->taille_contenu];
        sprintf(msg, "<%s> %s" , m->auteur , m->contenu );
        write(idSocket, msg, 6+m->taille_contenu );
    }
    listeparcours = listeparcours->prochain;
    if (listeMsg !=NULL )
        m = listeparcours->me;
   } 
}

liste_client creer_liste_client ()
{
    return malloc(sizeof(liste_client_s));
} 
 
void newClient(struct sockaddr_in adresse, char* pseudo, client c) {
    c->abonnements = NULL; 
    c->abonnes = NULL;
    c->derniereDeconnexion = 0; 
    c->addr = adresse ;
    c->pseudo = pseudo;
}

void insertListeClient (liste_client* listeClient, client c ) {
    liste_client newhead = malloc(sizeof(liste_client));
    newhead->cl = c;
    newhead->prochain = *listeClient;
    *listeClient = newhead;
}

client findClient(liste_client listeClient, char* pseudo ) {
   //start from the beginning
   while(listeClient != NULL) {
    client personne = listeClient->cl;
    printf("find client : %s\n", personne->pseudo);
    if (strcmp(personne->pseudo,pseudo)==0) { 
        return personne;
    }
    listeClient = listeClient->prochain;
   }
   return NULL;
}

client findClientfromAddr(liste_client listeClient, struct sockaddr_in adresse ) {
   //start from the beginning
   while(listeClient != NULL) {
    client personne = listeClient->cl;
    if (personne->addr.sin_addr.s_addr == adresse.sin_addr.s_addr) {
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
void addSubscription (client* c_follower, client* c_following) {
    liste_client newhead = malloc(sizeof(liste_client));
    newhead->cl = *c_follower;
    newhead->prochain = (*c_following)->abonnes;
    (*c_following)->abonnes = newhead;
    liste_client newhead2 = malloc(sizeof(liste_client));
    newhead2->cl = (*c_following);
    newhead2->prochain = (*c_follower)->abonnements;
    (*c_follower)->abonnements = newhead2;
}

void rmClient(liste_client *listeptr, client c) {
   liste_client liste = *listeptr;
   liste_client previousEntry = NULL;
   printf("rmClient : %s\n", c->pseudo);
   while(liste != NULL) {
    client personne = liste->cl;
    if (previousEntry != NULL && liste->cl == c) {
        previousEntry->prochain = liste->prochain;
        free(liste);
        break;
    } else if (liste->cl == c) {
        *listeptr = liste->prochain;
        free(liste);
        break;

    }
    previousEntry = liste;
    liste = liste->prochain;
   }
   return;
}

void removeSubscription (client c_follower, client c_following) {
    rmClient( &c_follower->abonnements, c_following);
    rmClient( &c_following->abonnes, c_follower);
}

void insertConnectedClients (connected_clients* liste, client c, int socket ) {
    connected_clients newhead = malloc(sizeof(connected_clients));
    newhead->cl = c;
    newhead->socket = socket;
    newhead->prochain = *liste;
    (*liste) = newhead;
}

client findConnectedClient(connected_clients liste, int socket) {
   while(liste != NULL) {
    client personne = liste->cl;
    if (liste->socket == socket) {
        return liste->cl;
    }
    liste = liste->prochain;
   }
   return NULL;
}

void rmConnectedClient(connected_clients *listeptr, int socket) {
   connected_clients liste = *listeptr;
   connected_clients previousEntry = NULL;
   while(liste != NULL) {
    client personne = liste->cl;
    if (previousEntry != NULL && liste->socket == socket) {
        previousEntry->prochain = liste->prochain;
        free(liste);
        break;
    } else if (liste->socket == socket) {
        *listeptr = liste->prochain;
        free(liste);
        break;

    }
    previousEntry = liste;
    liste = liste->prochain;
   }
   return;
}
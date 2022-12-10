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
#include <stdbool.h>
#include <unistd.h>
#include <time.h>

void find_date(long timestamp, int *date, int *hour){
    char buf[20];
    struct tm *time_struct = (struct tm*) timestamp;
    strftime(buf, sizeof(buf), "%Y%m%d,%H", time_struct);
    printf("Time: %s", buf);
}

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


char* writeNewMsg(liste_message* listeMsg, int idSocket, char* auteur, long date ) {
   char * messages = malloc(sizeof(char)*800); //taille max buffer reception client
   messages[0] = '\0'; //pr le strcat
   char * msg = malloc(sizeof(char)*(6+6+15+20)); //char entête + taillemax pseudo + taille date + taille max msg 
   char * time = malloc(15); //taille date 
   message m = (*listeMsg)->me;
   liste_message listeparcours = *listeMsg;
   printf("writeNewMsg: %s, %ld\n", m->contenu, m->date);
   //start from the beginning
   while(listeparcours != NULL && (m->date > date)) {
    if (strcmp(m->auteur,auteur)==0) {
        strftime(time, 15, "%a - %X", localtime(&m->date));
        sprintf(msg, "<%s | %s> %s\n" , m->auteur , time, m->contenu );
        strcat(messages, msg);
    }
    listeparcours = listeparcours->prochain;
    if (listeparcours !=NULL )
        m = listeparcours->me;
   } 
   return messages;
}

liste_client creer_liste_client ()
{
    return malloc(sizeof(liste_client_s));
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

char* getNamesFromListeClient(liste_client liste/*, bool isFollowing*/) {
    char* liste_pseudos = calloc(800, sizeof(char));; //not flexible...
    /*if(isFollowing)
        liste_pseudos = "Abonnements :";
    else
        liste_pseudos = "Abonnés :";*/
    while(liste != NULL) {
        client name = liste->cl;
        sprintf (liste_pseudos + strlen(liste_pseudos), " %s .", name->pseudo);
        liste = liste->prochain;
    }
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
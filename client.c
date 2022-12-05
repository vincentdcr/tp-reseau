/******************************************************************************/
/*			Application: Jeu du Pendu							              */
/******************************************************************************/
/*									     									  */
/*			 programme  CLIENT	 				      						  */
/*									      									  */
/******************************************************************************/
/*									      									  */
/*		Auteurs :  Benjamin Bracquier / Mathis Grange / Vincent Ducros    	  */
/*		Date :  29/04/22						      						  */
/*									      									  */
/******************************************************************************/


#include <stdio.h>
#include <curses.h> 		/* Primitives de gestion d'ecran */
#include <sys/signal.h>
#include <sys/wait.h>
#include<stdlib.h>
#include <ctype.h>
#include <string.h>

#include "fon.h"   		/* primitives de la boite a outils */

#define SERVICE_DEFAUT "1111"
#define SERVEUR_DEFAUT "127.0.0.1"
#define BUFFER_SIZE 80

void client_appli (char *serveur, char *service);


/*****************************************************************************/
/*--------------- programme client -----------------------*/

int main(int argc, char *argv[])
{

	char *serveur= SERVEUR_DEFAUT; /* serveur par defaut */
	char *service= SERVICE_DEFAUT; /* numero de service par defaut (no de port) */


	/* Permet de passer un nombre de parametre variable a l'executable */
	switch(argc)
	{
 	case 1 :		/* arguments par defaut */
		  printf("serveur par defaut: %s\n",serveur);
		  printf("service par defaut: %s\n",service);
		  break;
  	case 2 :		/* serveur renseigne  */
		  serveur=argv[1];
		  printf("service par defaut: %s\n",service);
		  break;
  	case 3 :		/* serveur, service renseignes */
		  serveur=argv[1];
		  service=argv[2];
		  break;
    default:
		  printf("Usage:client serveur(nom ou @IP)  service (nom ou port) \n");
		  exit(1);
	}

	/* serveur est le nom (ou l'adresse IP) auquel le client va acceder */
	/* service le numero de port sur le serveur correspondant au  */
	/* service desire par le client */
	
	client_appli(serveur,service);
}

/*****************************************************************************/
void client_appli (char *serveur,char *service)

/* procedure correspondant au traitement du client de votre application */

{
    /*===== Établissement de la connexion =====*/

	// Création d'une socket
    int id_client_socket = h_socket(AF_INET, SOCK_STREAM);

	struct sockaddr_in *p_adr_socket;
	adr_socket(service, serveur, SOCK_STREAM, &p_adr_socket);

	// Demande de connexion au serveur
    h_connect(id_client_socket, p_adr_socket);

    /*===== Jeu du pendu (comm. client/serveur) =====*/

    char *read_server =malloc(sizeof(char)*BUFFER_SIZE);
    char *read_client =malloc(sizeof(char)*10);
	read_client[0] = '1';
    while (read_client[0] != '0') {
		//reception d'une chaine de 80 caracteres depuis le serveur (et affichage)
        h_reads(id_client_socket, read_server, 80);
        printf("%s", read_server);

		//envoi d'UN caractere en majuscule au serveur (les autres sur la ligne = ignorés)
        scanf("%s", read_client);
        read_client[0] = toupper(read_client[0]);
        h_writes(id_client_socket, read_client, 1);
    }

    /*===== Fermeture de la connexion =====*/
    h_close(id_client_socket);
}

/*****************************************************************************/
/******************************************************************************/
/*			Application: Flash 									              */
/******************************************************************************/
/*									     									  */
/*			 programme  CLIENT	 				      						  */
/*									      									  */
/******************************************************************************/
/*									      									  */
/*		Auteurs :  Paul Grandhomme / Vincent Ducros  					  	  */
/*		Date :  5/12/22							      						  */
/*									      									  */
/******************************************************************************/


#include <stdio.h>
#include <curses.h> 		/* Primitives de gestion d'ecran */
#include <sys/signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>

#define SERVICE_DEFAUT "9999"
#define SERVEUR_DEFAUT "127.0.0.1"
#define BUFFER_SIZE 800

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
    /*===== Etablissement de la connexion =====*/

	// Creation d'une socket
    int id_client_socket = socket(AF_INET, SOCK_STREAM,IPPROTO_TCP);

	struct sockaddr_in p_adr_socket;

	p_adr_socket.sin_family = AF_INET;
    p_adr_socket.sin_port = htons(atoi(service)); // use some unused port number
    p_adr_socket.sin_addr.s_addr = INADDR_ANY;
	// Demande de connexion au serveur
    connect(id_client_socket, (struct sockaddr *) &p_adr_socket, sizeof(p_adr_socket));
    /*===== Service msg Flash (comm. client/serveur) =====*/

    char *read_server =malloc(sizeof(char)*BUFFER_SIZE);
    char *read_client =malloc(sizeof(char)*10);
	read_client[0] = '1';
    while (read_client[0] != '0') {
		//reception d'une chaine de 800 caracteres depuis le serveur (et affichage)
        read(id_client_socket, read_server, 80);
        printf("%s", read_server);
		//envoi d'UN caractere en majuscule au serveur (les autres sur la ligne = ignorés)
        scanf("%s", read_client);
        read_client[0] = toupper(read_client[0]);
		printf("avant write\n");
        write(id_client_socket, read_client, 1);
		printf("test %c\n", read_client[0]);
    }

    /*===== Fermeture de la connexion =====*/
    close(id_client_socket);
}

/*****************************************************************************/
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
#include <curses.h> /* Primitives de gestion d'ecran */
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
#include <sys/time.h>
 #include <fcntl.h>

#define SERVICE_DEFAUT "9999"
#define SERVEUR_DEFAUT "127.0.0.1"
#define BUFFER_SIZE 800

void client_appli(char *serveur, char *service);

/*****************************************************************************/
/*--------------- programme client -----------------------*/

int main(int argc, char *argv[])
{

	char *serveur = SERVEUR_DEFAUT; /* serveur par defaut */
	char *service = SERVICE_DEFAUT; /* numero de service par defaut (no de port) */

	/* Permet de passer un nombre de parametre variable a l'executable */
	switch (argc)
	{
	case 1: /* arguments par defaut */
		printf("serveur par defaut: %s\n", serveur);
		printf("service par defaut: %s\n", service);
		break;
	case 2: /* serveur renseigne  */
		serveur = argv[1];
		printf("service par defaut: %s\n", service);
		break;
	case 3: /* serveur, service renseignes */
		serveur = argv[1];
		service = argv[2];
		break;
	default:
		printf("Usage:client serveur(nom ou @IP)  service (nom ou port) \n");
		exit(1);
	}

	/* serveur est le nom (ou l'adresse IP) auquel le client va acceder */
	/* service le numero de port sur le serveur correspondant au  */
	/* service desire par le client */

	client_appli(serveur, service);
}

/*****************************************************************************/
void client_appli(char *serveur, char *service)

/* procedure correspondant au traitement du client de votre application */

{
	/*===== Etablissement de la connexion =====*/

	// Creation d'une socket
	int id_client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	struct sockaddr_in p_adr_socket;

	p_adr_socket.sin_family = AF_INET;
	p_adr_socket.sin_port = htons(atoi(service)); // use some unused port number
	p_adr_socket.sin_addr.s_addr = INADDR_ANY;
	// Demande de connexion au serveur
	if (connect(id_client_socket, (struct sockaddr *)&p_adr_socket, sizeof(p_adr_socket)) < 0)
	{
		perror("connect");
		exit(EXIT_FAILURE);
	}
	/*===== Service msg Flash (comm. client/serveur) =====*/

	/* Initialize the set of active sockets. */
	fd_set active_fd_set, read_fd_set, write_fd_set;
	FD_ZERO(&active_fd_set);
	FD_SET(STDIN_FILENO, &active_fd_set);
	FD_SET(id_client_socket, &active_fd_set);

	char *read_server = malloc(sizeof(char) * BUFFER_SIZE);
	char *read_client = malloc(sizeof(char) * 27);
	read_client[0] = '1';
	struct timeval delai = {10, 0};

	fcntl(id_client_socket, F_SETFL, O_NONBLOCK);
	fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
	while (read_client[0] != 'q')
	{
		/* Block until input arrives on one or more active sockets. */
		read_fd_set = active_fd_set;
		if (select(id_client_socket+1, &read_fd_set, NULL, NULL, &delai) < 0)
		{
			perror("select");
			exit(EXIT_FAILURE);
		}
		/* Service all the sockets with input pending. */
		if (FD_ISSET(STDIN_FILENO, &read_fd_set))
		{
			int nbchar = read(STDIN_FILENO, read_client, 27);
			if (nbchar >=0)
				read_client[nbchar] = '\0';
			write(id_client_socket, read_client, 27);
		}
		if (FD_ISSET(id_client_socket, &read_fd_set))
		{
			read(id_client_socket, read_server, BUFFER_SIZE);
			printf("%s\n", read_server);
		}
	}

	/*===== Fermeture de la connexion =====*/
	FD_CLR(STDOUT_FILENO, &active_fd_set);
	close(id_client_socket);
}



/*****************************************************************************/
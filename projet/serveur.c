/******************************************************************************/
/*			Application: Flash									              */
/******************************************************************************/
/*									     									  */
/*			 programme  SERVEUR 				      						  */
/*									      									  */
/******************************************************************************/
/*									      									  */
/*		Auteurs :  Paul Grandhomme / Vincent Ducros					    	  */
/*		Date :  5/12/22							      						  */
/*									      									  */
/******************************************************************************/

#include <stdio.h>
#include <curses.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <sys/signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include  <sys/time.h>
#include "types.h"


#define SERVICE_DEFAUT "9999"

#define STDIN 0 // Standard input (stdin)
#define BUFFER_SIZE 80


void serveur_appli(char *service); /* programme serveur */

/******************************************************************************/
/*---------------- programme serveur ------------------------------*/

int main(int argc, char *argv[])
{

	char *service = SERVICE_DEFAUT; /* numero de service par defaut */

	/* Permet de passer un nombre de parametre variable a l'executable */
	switch (argc)
	{
	case 1:
		printf("defaut service = %s\n", service);
		break;
	case 2:
		service = argv[1];
		break;

	default:
		printf("Usage:serveur service (nom ou port) \n");
		exit(1);
	}

	/* service est le service (ou numero de port) auquel sera affecte
	ce serveur*/
	serveur_appli(service);
}


/*Fct pour completer le buffer et l'emmener jusqu'a une taille fixe de BUFFER_SIZE caracteres pour toujours envoyer un paquet de meme taille
@param : chaine contenant les infos a transmettre (taille < BUFFER_SIZE)
@res : chaine de BUFFER_SIZE caracteres a transmettre*/
char * completemsg(char* buffer) {
	char * msg = malloc(sizeof(char)*BUFFER_SIZE);
	int taille = strlen(buffer);
	//copie du msg dans le debut de la chaine
	for(int i=0 ; i<taille; i++)
		msg[i]=buffer[i];
	// on complete par des espaces puis par un retour a la ligne
	for (int j=taille; j<(BUFFER_SIZE-1) ; j++)
		msg[j]=' ';
	msg[BUFFER_SIZE-1]='\n';
	return msg;
}

/******************************************************************************/
void serveur_appli(char *service)
/* Procedure correspondant au traitemnt du serveur de votre application */

{
	//===== Etablissement connexion ======
	//création d'une socket
	int IDsocket_passif = socket(AF_INET, SOCK_STREAM,IPPROTO_TCP);
	setsockopt(IDsocket_passif, SOL_SOCKET ,SO_REUSEADDR, "1", 1);
	struct sockaddr_in p_adr_socket;
 	p_adr_socket.sin_family = AF_INET;
	p_adr_socket.sin_port = htons(atoi(service));
	p_adr_socket.sin_addr.s_addr = INADDR_ANY;

	//association socket avec num socket du serveur
	if ( bind(IDsocket_passif, (struct sockaddr *) &p_adr_socket, sizeof(p_adr_socket) ) < 0 )
        {
          perror ("bind");
          exit (EXIT_FAILURE);
        }
	//serveur en attente d'1 connexion
	if ( listen(IDsocket_passif, 1) < 0 )
        {
          perror ("listen");
          exit (EXIT_FAILURE);
        }
 

	/* Initialize the set of active sockets. */
	fd_set active_fd_set, read_fd_set;
  	FD_ZERO (&active_fd_set);
  	FD_SET (IDsocket_passif, &active_fd_set);	

    //===== Flash (comm. client/serveur) =====

	char * buffer = malloc(sizeof(char)*BUFFER_SIZE);
	int Flag = 1;
	liste_client listeClients = creer_liste_client();
	liste_message listeMsg = creer_liste_messages(); 

	while (Flag)
    {
      /* Block until input arrives on one or more active sockets. */
      read_fd_set = active_fd_set;
      if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0)
        {
          perror ("select");
          exit (EXIT_FAILURE);
        }
      /* Service all the sockets with input pending. */
      for (int i = 0; i < FD_SETSIZE; ++i)
        if (FD_ISSET (i, &read_fd_set))
          {
            if (i == IDsocket_passif)
              {
                /* Connection request on original socket. */
				printf("test serveur\n");
                int IDsocket_client = accept(IDsocket_passif,NULL, NULL );
                if (IDsocket_client < 0)
                  {
                    perror ("accept");
                    exit (EXIT_FAILURE);
                  }
                FD_SET (IDsocket_client, &active_fd_set);
				write(IDsocket_client,"Enter command (a,d,l,m,n,h,q) :", 32);
              }
            else
              {
  				if (read (i, buffer, BUFFER_SIZE) < 0)
    			{
      				/* Read error. */
      				perror ("read");
      				exit (EXIT_FAILURE);
    			}
				/* Data read. */
      			printf ("Server: got message: '%s'\n", buffer);
  				switch (buffer[0]) {
					case 'a':
						break;
					case 'd':
						break;
					case 'l':
						break;
					case 'm':
						break;
					case 'n':
						break;
					case 'h':
						break;
					case 'q':	
						close(i);
						FD_CLR (i, &active_fd_set);
    					Flag = 0;
						break;
				}
				write(i,"Enter command (a,d,l,m,n,h,q) :", 32);
              }
          }
    }
	
    /*===== Fermeture de la connexion =====*/
	close(IDsocket_passif);
	FD_CLR (IDsocket_passif, &active_fd_set);
}

/******************************************************************************/

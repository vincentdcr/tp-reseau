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
void printAllNewMessages(liste_message listeMsg, client c, int socket);

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
	//serveur en attente de 5 connexions
	if ( listen(IDsocket_passif, 5) < 0 )
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
	liste_client listeClients = NULL;
	liste_message listeMsg = NULL; 
	connected_clients listeConnected = NULL;
	client c;
	client following;

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
				struct sockaddr_in addr_client;
				socklen_t addr_client_size = sizeof(addr_client);
                int IDsocket_client = accept(IDsocket_passif,(struct sockaddr *) &addr_client, &addr_client_size );
                if (IDsocket_client < 0)
                  {
                    perror ("accept");
                    exit (EXIT_FAILURE);
                  }
                FD_SET (IDsocket_client, &active_fd_set);
				c = findClientfromAddr(listeClients, addr_client);
				if (c == NULL) 
				{
					write(IDsocket_client,"Enter pseudo (max 6 chars)", 27);
  					if (read (IDsocket_client, buffer, BUFFER_SIZE) < 0)
    				{
      					/* Read error. */
      					perror ("read");
      					exit (EXIT_FAILURE);
    				}
					char* pseudo = malloc(7*sizeof(char));
					strcpy(pseudo, buffer);
					client_s client = { pseudo, 0, addr_client, NULL, NULL};
					insertListeClient(&listeClients, &client);
					insertConnectedClients(&listeConnected, &client, IDsocket_client);
				} else {
					sprintf(buffer, "Welcome back %s\nEnter command (a,d,l,m,n,h,q) :", c->pseudo );
					printAllNewMessages(listeMsg, c, IDsocket_client);
					write(IDsocket_client, buffer, 22);
				}
				write(IDsocket_client,"Enter command (a,d,l,m,n,h,q) :", 32);
              }
            else
              {
				c = findConnectedClient(listeConnected, i);
  				if (read (i, buffer, BUFFER_SIZE) < 0)
    			{
      				/* Read error. */
      				perror ("read");
      				exit (EXIT_FAILURE);
    			}
				/* Data read. */
      			printf ("Server: got message: '%s' of length %ld\n", buffer, strlen(buffer));
  				switch (buffer[0]) {
					case 'a':
						write(i,"Essai abonnement", 22);
					  	if (read (i, buffer, BUFFER_SIZE) < 0)
    					{		/* Read error. */
      						perror ("read");
      						exit (EXIT_FAILURE);
    					}
						following = findClient(listeClients, buffer);
						if (following != NULL)
							addSubscription(&c, &following); 
						else 
							write(i, "Cet abonné n'existe pas", 25);
						break;
					case 'd':
						write(i,"Essai désabonnement", 22);
						if (read (i, buffer, BUFFER_SIZE) < 0)
    					{		/* Read error. */
      						perror ("read");
      						exit (EXIT_FAILURE);
    					}
						following = findClient(listeClients, buffer);
						if (following != NULL)
							removeSubscription(c, following); 
						else 
							write(i, "Cet abonné n'existe pas", 25);
						break;
					case 'l': 
						break;
					case 'm':
						insertListeMsg(&listeMsg, newMessage(buffer, BUFFER_SIZE, time(NULL), c->pseudo));
						break;
					case 'n':
						break;
					case 'h':
						write(i,"Commands :\na <pseudo> : s'abonner\nd <pseudo> : se désabonner\nl : lister abo\nm <msg> : ecrire msg\nh : aide comm.\nq : quitter\n\nEnter command (a,d,l,m,n,h,q) :", 158);
						break;
					case 'q':	
						close(i);
						FD_CLR (i, &active_fd_set);
						rmConnectedClient(&listeConnected, i);
    					//Flag = 0;
						//break;
				}
				write(i,"Enter command (a,d,l,m,n,h,q) :", 32);
              }
          }
    }
	
    /*===== Fermeture de la connexion =====*/
	close(IDsocket_passif);
	FD_CLR (IDsocket_passif, &active_fd_set);
}


void printAllNewMessages(liste_message listeMsg, client c, int socket) {
	liste_client subscriptions = c->abonnements;
	while (subscriptions != NULL) {
		writeNewMsg(listeMsg, socket, subscriptions->cl->pseudo , c->derniereDeconnexion );
		subscriptions =  subscriptions->prochain;
	}
}

/******************************************************************************/

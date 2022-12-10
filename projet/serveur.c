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
#define BUFFER_SIZE 27


void serveur_appli(char *service); /* programme serveur */
void printAllNewMessages(liste_message listeMsg, client c, int socket);
char * getArgument(char * command, bool flagCheckSpaces);
char* getName(char * command);

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
	char * argument = malloc(sizeof(char)*(BUFFER_SIZE-2));
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
					char* pseudo = malloc(sizeof(char)*7);
					pseudo = getName(buffer);
					printf("pseudo : %s\n", pseudo);
					client_s client = { pseudo, 0, addr_client, NULL, NULL};
					insertListeClient(&listeClients, &client);
					insertConnectedClients(&listeConnected, &client, IDsocket_client);
				} else {
					sprintf(buffer, "Welcome back %s\nEnter command (a,d,l,m,n,h,q) :", c->pseudo );
					printAllNewMessages(listeMsg, c, IDsocket_client);
					write(IDsocket_client, buffer, 46+strlen(c->pseudo));
				}
				write(IDsocket_client,"Entrer command (a,d,l,m,n,h,q) :", 33);
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
					case 'a': {
						write(i,"Essai abonnement", 17);
						argument = getArgument(buffer, TRUE);
						following = findClient(listeClients, argument);
						if (following != NULL)
							addSubscription(&c, &following); 
						else 
							write(i, "Cet abonné n'existe pas", 25);
						break;
					}
					case 'd': {
						write(i,"Essai désabonnement", 21);
						argument = getArgument(buffer, TRUE);
						following = findClient(listeClients, argument);
						if (following != NULL)
							removeSubscription(c, following); 
						else 
							write(i, "Cet abonné n'existe pas", 25);
						break;
					}
					case 'l': {
						break;
					}
					case 'm': {
						write(i,"Essai envoi message", 20);
						argument = getArgument(buffer, FALSE);
						message_s m1;
						message m = (message) malloc(sizeof(message_s));
						m->contenu = argument;
						m->taille_contenu = strlen(argument);
						m->date = time(NULL);
						m->auteur = c->pseudo; 
						insertListeMsg(&listeMsg,m);
						break;
					}
					case 'n': {
						printAllNewMessages(listeMsg, c, i);
						break;
					}
					case 'h': {
						write(i,"Commands :\na <pseudo> : s'abonner\nd <pseudo> : se désabonner\nl : lister abo\nm <msg> : ecrire msg\nh : aide comm.\nq : quitter\n\nEnter command (a,d,l,m,n,h,q) :", 158);
						break;
					}
					case 'q': {	
						close(i);
						FD_CLR (i, &active_fd_set);
						rmConnectedClient(&listeConnected, i);
    					//Flag = 0;
						//break;
					}
				}
				write(i,"Entrer command (a,d,l,m,n,h,q) :", 33);
              }
          }
    }
	
    /*===== Fermeture de la connexion =====*/
	close(IDsocket_passif);
	FD_CLR (IDsocket_passif, &active_fd_set);
}


void printAllNewMessages(liste_message listeMsg, client c, int socket) {
	liste_client subscriptions = c->abonnements;
	char * messages = malloc(sizeof(char)*800); //pas bien mais comment faire ?
	while (subscriptions != NULL) {
		printf("print msg : %s , %ld\n", subscriptions->cl->pseudo, c->derniereDeconnexion);
		messages = writeNewMsg(&listeMsg, socket, subscriptions->cl->pseudo , c->derniereDeconnexion );
		write(socket, messages, strlen(messages));
		subscriptions =  subscriptions->prochain;
	}
}

char * getArgument(char * command, bool flagCheckSpaces) {
	char * argument = malloc(sizeof(strlen(command)-2));
	char * temp = command;
	int j=0;
	for(int i=2; i<strlen(command); i++) {
		if ((temp[i]==' ' && flagCheckSpaces) || temp[i]=='\n')
			break;
		argument[j]=temp[i];
		j++;
	}
	return argument;
}

char * getName(char * command) {
	char * name = malloc(sizeof(strlen(command)-2));
	char * temp = command;
	int j=0;
	for(int i=0; i<strlen(command); i++) {
		if (temp[i]=='\n')
			break;
		name[j]=temp[i];
		j++;
	}
	return name;
}


/******************************************************************************/

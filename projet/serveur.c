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
#include <unistd.h>
#include  <sys/time.h>
#include "types.h"
 #include <fcntl.h>


#define SERVICE_DEFAUT "9998"

#define STDIN 0 // Standard input (stdin)
#define BUFFER_SIZE 24


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
/* Procedure correspondant au traitement du serveur de l'application */

{
	//===== Etablissement connexion ======
	//création d'une socket
	int IDsocket_passif = socket(AF_INET, SOCK_STREAM,IPPROTO_TCP);
	setsockopt(IDsocket_passif, SOL_SOCKET ,SO_REUSEADDR, "1", 1);
	struct sockaddr_in p_adr_socket;
 	p_adr_socket.sin_family = AF_INET; //IPv4
	p_adr_socket.sin_port = htons(atoi(service));	//service = Num. Port
	p_adr_socket.sin_addr.s_addr = INADDR_ANY;

	//association socket avec num socket du serveur
	if ( bind(IDsocket_passif, (struct sockaddr *) &p_adr_socket, sizeof(p_adr_socket) ) < 0 )
        {
          perror ("bind");
          exit (EXIT_FAILURE);
        }
	//serveur en attente de 10 connexions max
	if ( listen(IDsocket_passif, 10) < 0 )
        {
          perror ("listen");
          exit (EXIT_FAILURE);
        }

    //===== Flash (comm. client/serveur) =====

	/* Initialisation des descripteurs de fichiers. */
	fd_set active_fd_set;
  	FD_ZERO (&active_fd_set);
  	FD_SET (IDsocket_passif, &active_fd_set);	
	fcntl(IDsocket_passif, F_SETFL, fcntl(IDsocket_passif, F_GETFL, 0) | O_NONBLOCK); //on rend le socket non-bloquant

	/* Initialisation des buffers */
	char * buffer = malloc(sizeof(char)*BUFFER_SIZE);
	char * argument = malloc(sizeof(char)*(BUFFER_SIZE-2)); /* buffer de l'argument de la ln de comm. */

	// Drapeau mis à faux pour fermer le serveur.
	int Flag = 1;

	// Init des linked listes définit dans types.h
	liste_client listeClients = NULL;
	liste_message listeMsg = NULL; 
	connected_clients listeConnected = NULL;

	client c = NULL;
	client following;

	while (Flag)
    {
      	/* Faire tourner le code pour chaque fd*/
      	for (int i = 0; i < FD_SETSIZE; ++i) {
			if (FD_ISSET (i, &active_fd_set))
          	{
				if (i == IDsocket_passif)
				{ 
					/* Requête de connexion sur le serveur*/
					struct sockaddr_in addr_client;
					socklen_t addr_client_size = sizeof(addr_client);
					int IDsocket_client = accept(IDsocket_passif,(struct sockaddr *) &addr_client, &addr_client_size );
					if (IDsocket_client >= 0) 
					{

						FD_SET (IDsocket_client, &active_fd_set);
						//c = findClientfromAddr(listeClients, addr_client);
						write(IDsocket_client,"Enter pseudo (max 6 chars)", 27);
						if (read (IDsocket_client, buffer, BUFFER_SIZE) < 0)
						{
							/* Read error. */
							perror ("read");
							exit (EXIT_FAILURE);
						}
						fcntl(IDsocket_client, F_SETFL, fcntl(IDsocket_client, F_GETFL, 0) | O_NONBLOCK); //on passe le fd en non-bloquant
						char* pseudo = malloc(sizeof(char)*7);
						pseudo = getName(buffer);
						c = findClient(listeClients, pseudo);
						if (c == NULL) {
							printf("New client connecting : %s\n", pseudo);
							client cli = (client) malloc(sizeof(client_s));
							cli->pseudo = pseudo;
							cli->derniersMsgLus = 0;
							cli->addr = addr_client;
							cli->abonnements = NULL;
							cli->abonnes = NULL;
							insertListeClient(&listeClients, cli);
							insertConnectedClients(&listeConnected, cli, IDsocket_client);
						} else {
							printf("Returning client connecting : %s\n", pseudo);
							insertConnectedClients(&listeConnected, c, IDsocket_client);
							sprintf(buffer, "Welcome back %s", c->pseudo );
							write(IDsocket_client, buffer, 14+strlen(c->pseudo));
						}
						write(IDsocket_client,"Entrer commande (a,d,l,m,h,q) :", 32);
					}
				}
				else
				{
					c = findConnectedClient(listeConnected, i);
					printAllNewMessages(listeMsg, c, i);
					if (read (i, buffer, BUFFER_SIZE) > 0)
					{
						/* Data read. */
						printf ("Server: got message: '%s' of length %ld\n", buffer, strlen(buffer));
						switch (buffer[0]) {
							case 'a': {
								argument = getArgument(buffer, FALSE);
								following = findClient(listeClients, argument);
								// si le pseudo existe et qu'on est pas déjà abonné
								if (following != NULL && findClient(c->abonnements, argument)==NULL ) {
									addSubscription(&c, &following); 
									write(i, "Abonnement ajouté", 19);
								} else 
									write(i, "Cet abonné n'existe pas / déjà abonné", 42);
								break;
							}
							case 'd': {
								argument = getArgument(buffer, FALSE);
								following = findClient(listeClients, argument);
								// si le pseudo existe et qu'on est déjà abonné
								if (following != NULL && findClient(c->abonnements, argument)!=NULL) {
									removeSubscription(c, following); 
									write(i, "Désabonnement confirmé", 25);
								} else 
									write(i, "Cet abonné n'existe pas / pas abonné", 25);
								break;
							}
							case 'l': {
								/* Meilleur affichage abonnements/abonnés : (marche pas tout a fait)
								
								char * listePseudos = getNamesFromListeClient(c->abonnements, TRUE);
								write(i, listePseudos, strlen(listePseudos)+1); // pour le \0 final

								listePseudos = getNamesFromListeClient(c->abonnes, FALSE);
								write(i, listePseudos, strlen(listePseudos)+1); // pour le \0 final */
								char * listePseudos = getNamesFromListeClient(c->abonnements);
								write(i, listePseudos, strlen(listePseudos)+1); // pour le \0 final
								break;
							}
							case 'm': {
								argument = getArgument(buffer, FALSE);
								message m = (message) malloc(sizeof(message_s));
								m->contenu = argument;
								m->taille_contenu = strlen(argument);
								m->date = time(NULL);
								m->auteur = c->pseudo; 
								insertListeMsg(&listeMsg,m);
								write(i, "Message ajouté", 25);
								break;
							}
							case 'h': {
								write(i,"Commandes :\na <pseudo> : s'abonner\nd <pseudo> : se désabonner\nl : lister abo\nm <msg> : ecrire msg\nh : aide comm.\nq : quitter\n", 127);
								break;
							}
							case 'q': {	
								close(i);
								FD_CLR (i, &active_fd_set);
								rmConnectedClient(&listeConnected, i);
								//Flag = 0;
								break;
							}
						}
						write(i,"Entrer commande (a,d,l,m,h,q) :", 32);
					}	
				}
			}
        }
    }
	
    /*===== Fermeture de la connexion =====*/
	close(IDsocket_passif);
	FD_CLR (IDsocket_passif, &active_fd_set);
}


void printAllNewMessages(liste_message listeMsg, client c, int socket) {
	if (listeMsg != NULL) {
		bool hasReadNewMsg = false;
		liste_client subscriptions = c->abonnements;
		char * messages = malloc(sizeof(char)*800); //pas extensible si trop de msg mais comment faire ?
		while (subscriptions != NULL) {
			writeNewMsg(messages, &listeMsg, socket, subscriptions->cl->pseudo , c->derniersMsgLus );
			if (strlen(messages)!=0) { //pr ne pas réafficher des msg déjà lus
				printf("print all msg from %s from time %ld\n %s ", subscriptions->cl->pseudo, c->derniersMsgLus, messages);
				write(socket, messages, strlen(messages)+1); // +1 pour le \0 final
				hasReadNewMsg = true;
			}
			subscriptions =  subscriptions->prochain;
		}
		if (hasReadNewMsg)
			c->derniersMsgLus = time(NULL);
		free(messages);
	}
}
//recupere l'argument d'une commande envoye par un client
char * getArgument(char * command, bool flagCheckSpaces) {
	char * argument = calloc(strlen(command)-2,sizeof(char)); //sinon on ajoute jamais de \0
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
//recupere le pseudo en enlevant le \n final rajoute par le write
char * getName(char * command) {
	char * name = calloc(strlen(command)-2,sizeof(char));
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

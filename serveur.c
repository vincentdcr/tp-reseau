/******************************************************************************/
/*			Application: Jeu du Pendu							              */
/******************************************************************************/
/*									     									  */
/*			 programme  SERVEUR 				      						  */
/*									      									  */
/******************************************************************************/
/*									      									  */
/*		Auteurs :  Benjamin Bracquier / Mathis Grange / Vincent Ducros    	  */
/*		Date :  29/04/22						      						  */
/*									      									  */
/******************************************************************************/

#include <stdio.h>
#include <curses.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <sys/signal.h>
#include <sys/wait.h>
#include <stdlib.h>

#include "fon.h" /* Primitives de la boite a outils */

#define SERVICE_DEFAUT "1111"

#define STDIN 0 // Standard input (stdin)
#define BUFFER_SIZE 80

#define DICO_NB_MOTS 30
char dico[DICO_NB_MOTS][16] = { "CIBLE", "PARAGRAPHE", "POTABLE", "VIVANT", "PUZZLE", "CARTOGRAPHIE", "BRASSE", 
"REPTILE", "NORD", "COCCINELLE", "ETAT", "PERSIL", "CIMENT", "BANANE", "MINIGOLF", "VOITURE", "RECHERCHE",
"TRAPPE", "MAGASIN", "PROGRAMME", "PENDU", "SOMMEIL", "AIMANT", "PERPENDICULAIRE", "PERDU", "MUSICIEN", "DOS",
"PORTABLE", "GOUDRON", "PASTIS" };

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


/*Fct booleenne pour savoir si on a gagne et donc sortir de la boucle de jeu
(une des conditions pour savoir si gagne et d'avoir "decouvert" ts les caracteres du mot a chercher)
@param : word
@res : booleen FAUX pour sortir de la boucle de jeu*/
int isWin(char *wd) {
	for(int i=0; i<strlen(wd); i++) {
		if(wd[i]=='_') 
			return 1;
	}
	return 0;		
}

/*Fct pour changer l'etat de decouverte des caracteres dans la chaine transmise au client
@param : caractere a mettre a jour, chaine/mot (pr verif si car dans mot), chaine transmise
@res : pas de retour, on met a jour la chaine en mem*/
void is_char_in_word(char c, char * wd, char* word_display) {
	for (int i=0; i<strlen(wd); i++) {
		if (wd[i]==c)
			word_display[i]=c;
	}
}

/*Fct pour verif si la lettre fait partie de l'alphabet latin et si elle n'a pas deja ete testee
@param : liste des lettres testees, lettre a verifier, nombre de lettres testees (donc deja dans la liste)
@res : pas de retour, on met a jour la chaine en mem*/
int is_char_new(char *list_of_letter, char letter, int * index_last_letter) {
	if (letter<'A' || letter>'Z')
		return 0;
	for(int i=0; i<*index_last_letter; i++) {
		// le caractere n'est pas nouveau si on le trouve dans la liste
		if (list_of_letter[i]==letter) {
			return 0;
		}
	}
	//le caractere est nouveau, on le rajoute a la liste et on renvoit vrai
	list_of_letter[*index_last_letter]=letter;
	(*index_last_letter)++;
	return 1;

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
	int IDsocket_passif = h_socket(AF_INET, SOCK_STREAM);

	struct sockaddr_in *p_adr_socket;
	adr_socket(service, NULL, SOCK_STREAM, &p_adr_socket);

	//association socket avec num socket du serveur
	h_bind(IDsocket_passif, p_adr_socket);

	//serveur en attente d'1 connexion
	h_listen(IDsocket_passif, 1);

	//on accepte requête entrante et on définit un num de socket associé au client
	int IDsocket_client = h_accept(IDsocket_passif, p_adr_socket);

    //===== Jeu du pendu (comm. client/serveur) =====

	// init generateur aleatoire
	srand(clock());

	char * buffer = malloc(sizeof(char)*BUFFER_SIZE);
	// permet de rejouer tant que l'utilisateur le veut
	int Flag = 1;

	while (Flag)
	{

		// choix du mot a trouver
		int nrand = rand() % DICO_NB_MOTS;
		char *wd_to_find = dico[nrand];

		int size_wd = strlen(wd_to_find);
		//init du mot pour l'affichage client et déterminer la victoire
		char * word_display=malloc(sizeof(char)*size_wd);
		for (int i=0; i<size_wd; i++) 
			word_display[i]='_';

		//init liste lettres déjà entrées
		char list_of_letter[26];
		int index_last_letter = 0;

		//on envoit tjs un msg de BUFFER_SIZE octets
		h_writes(IDsocket_client,completemsg("Bienvenue dans le jeu du pendu\nNiveau de la partie (1 facile, 4 extrême)?"), BUFFER_SIZE);

		//choix du niveau

		h_reads(IDsocket_client, buffer, 1);
		int niveau_partie = atoi(buffer);
		//on boucle tant qu'on a pas un niveau attendu
		while(niveau_partie<1 || niveau_partie>4)	{
			h_writes(IDsocket_client,completemsg("Rentrez 1, 2, 3 ou 4 :"),BUFFER_SIZE);
			h_reads(IDsocket_client, buffer, 1);
			niveau_partie = atoi(buffer);
			printf("Niveau : %d\n", niveau_partie);
		}

		//Affichage serveur du niveau choisi
		printf("Niveau : %d\n", niveau_partie);
		//Affichage serveur du mot
		printf("Mot à chercher : %s\n", wd_to_find);

		//nombre d'essais pour trouver le mot
		int nb_tries;
		if (niveau_partie==1)
			nb_tries = (2 * size_wd) ;
		else if (niveau_partie==2)
			nb_tries = (1.6f * (float) size_wd) ;	
		else if (niveau_partie==3)
			nb_tries = (1.3f * (float) size_wd) ;
		else 
			nb_tries = size_wd;

		// boucle de jeu : on en sort si on gagne ou si on a epuise ts nos essais
		while (nb_tries > 0 && isWin(word_display))
		{
			//on génère dans le buffer la phrase qu'on va envoyer 
			sprintf(buffer, "Vous avez %d essais pour trouver le mot.\n%s", nb_tries, word_display);
			h_writes(IDsocket_client,completemsg(buffer), BUFFER_SIZE);
			
			//reception d'une lettre pour le pendu
			h_reads(IDsocket_client,buffer,1);
			while (!is_char_new(list_of_letter,buffer[0],&index_last_letter)) { // si pas une lettre / déjà essayé on redemande ss consommer un essai
				sprintf(buffer, "Vous avez déjà essayé %c (ou caractère invalide)", buffer[0]);
				h_writes(IDsocket_client,completemsg(buffer), BUFFER_SIZE);
				//reception d'une lettre pour le pendu
				h_reads(IDsocket_client, buffer, 1);
			}
			is_char_in_word(buffer[0], wd_to_find, word_display); //on modifie le mot afficher au client si la lettre reçue est dedans
			nb_tries--;
		}


		if (!isWin(word_display)) { // aff msg d'echec/reussite en fct de la decouverte ou non du mot
			sprintf(buffer, "Bravo vous avez gagné ! Le mot était : %s.\nVoulez-vous rejouer ? (1/0)",wd_to_find);
			h_writes(IDsocket_client,completemsg(buffer), BUFFER_SIZE);
		} else {
			sprintf(buffer, "Dommage, c'est perdu ! Le mot était : %s.\nVoulez-vous rejouer ? (1/0)", wd_to_find);
			h_writes(IDsocket_client,completemsg(buffer), BUFFER_SIZE);			
		}

		// relancer une partie
		h_reads(IDsocket_client,buffer,1);
		while (buffer[0] != '1' && buffer[0] != '0')
		{
			h_writes(IDsocket_client,completemsg("Rentrez 1/0 :"),BUFFER_SIZE);
			h_reads(IDsocket_client, buffer, 1);
		}
		if (buffer[0]== '0')
			Flag = 0; //on sort de la boucle principale
	}

    /*===== Fermeture de la connexion =====*/
	h_close(IDsocket_client);
	h_close(IDsocket_passif);
}

/******************************************************************************/

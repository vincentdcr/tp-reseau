//Type message et une liste chainee de messages

struct {
    char* contenu;
    int taille_contenu;
    long date;
    char* auteur;
} message_s;
typedef struct message_s *message;

struct {
    message me;
    struct liste_message_s* prochain;
} liste_message_s;
typedef struct liste_message_s *liste_message;

//Type client et une liste chainee de clients

struct {
    char pseudo[6];
    long derniereDeconnexion;
    int port; // IP ?
    struct liste_client_s* abonnements;
    struct liste_client_s* abonnes;
} client_s;

struct {
    struct client_s* cl;
    struct liste_client_s* prochain;
} liste_client_s;

typedef struct client_s *client;
typedef struct liste_client_s *liste_client;
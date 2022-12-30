##### Grandhomme Paul / Ducros Vincent

# TP4 RX

## Introduction

Dans ce TP on va concevoir une application d’échange de
messages courts via une interface serveur et une autre cliente. Pour cela, on va utiliser des sockets.

## Architecture du code

Le code est décomposé en 3 fichiers .c, dont deux exécutables.

- **client.c** : un code simple qui gère : 
    + création de la socket du client
    + l'ouverture de la connexion avec le serveur
    + une boucle infinie avec à l'intérieur la primitive *select* pour : 
        - afficher les messages en provenance du serveur (messages en temps réels pour les abonnements ou retour du serveur après l'exécution d'une commande)
        - envoyer les commandes entrées par l'utilisateur à celui-ci
    + la fermeture de la connexion
- **serveur.c** : ici on n'utilise pas select, qui bloquerait dans le cas où le client n'envoit pas de commande. A la place on définit tout nos sockets comme non-bloquant via : `fcntl(IDsocket_client, F_SETFL, fcntl(IDsocket_client, F_GETFL, 0) | O_NONBLOCK);`. Voilà les grandes lignes du code de ce fichier :
    + création de la socket du serveur
    + une boucle infinie qui vérifie chacun des sockets et effectue une action si possible:
        + si c'est la socket du serveur, vérifier si on peut établir une connexion, récuperer l'id de socket client (accept) et déterminer si c'est un nouveau client ou non (en fonction du pseudo).
        + si c'est la socket d'un client, lui envoyer les potentiels nouveaux messages, puis vérifier si il nous a envoyé une commande et l'exécuter

- **types.c** : On définit dans ce fichiers toutes les structures de données nécessaires et leurs fonctions d'accès / modification pour la gestion des clients et des messages par le serveur. On a choisit d'utiliser des listes chaînées pour la flexibilité qu'elles apportent et leur empreinte mémoire légère.

Il y a également un fichier **serveur_select.c**, il s'agit d'une ancienne version du code du serveur qui utilise la primitive *select*. On l'a gardé car le sujet précisé d'utiliser select pour le serveur et le client, mais on arrivait pas à récupérer les messages instantanément au moment où ils étaient pas publié via cette méthode.

## Cahier des charges

La connexion s'effectue via TCP, pour cela on fournit un numéro de port au serveur et on donne ce dernier ainsi que l'adresse du serveur au client.

Les échanges client/serveur lors du jeu sont décrits ci-dessous : 
##### Initialisation
Lorsque la connexion est établit, le serveur demande au client de rentrer un pseudo. Si celui-ci est reconnu (quelqu'un s'était déjà connecté avec ce pseudo), le serveur envoit un message *"Welcome back (pseudo)"* et le client est par la suite identifié comme étant cette personne (message envoyé en son nom, abonnements etc...).

##### Commandes à disposition du client

Au début puis à chaque fois que le client envoie une commande, le serveur lui renvoie "Entrer commande (a,d,l,m,h,q) :" pour l'informer de ses options. Dans l'ordre : 
- a [pseudo] : s'abonner à un autre utilisateur (renverra un message pour confirmer l'abonnement ou l'échec de celui-ci)
- d [pseudo] : se désabonner (renverra un message pour confirmer/infirmer celui-ci)
- l : lister les abonnements de l'utilisateur
- m [msg] : ecrire un message qui sera envoyé à tous les abonnés de l'utilisateur
- h : affiche une aide pour expliquer les commandes
- q : quitte et ferme la connexion avec le serveur

De plus, dès qu'un message est envoyé par un des utilisateur auquel on est abonné, on reçoit immédiatement celui-ci.


## Choix d'implémentations, contraintes non-respectés

Contrairement à ce qui avait été demandé, on a choisi par simplicité de stocker indéfiniment tous les messages sur le serveur tant que ce dernier tourne. Il n'y a également pas de mécanique de sauvegarde, si le serveur s'arrête, toutes les données sont perdues.

Il n'y a également pas de vérification effectuée sur la taille des messages (20 caractères max) ni des pseudos (6 caractères).

Enfin, on ne gère pas correctement les cas où la commande est bonne, mais l'espace entre celle-ci et son argument n'est pas respectée, par exemple taper : `message` depuis un client inscrira le message `ssage` sur le serveur. Cela vient de la façon dont sont écrites la fonction `getArgument` dans *serveur.c* .


## Démonstration d'une exécution

Lancer ./serveur et 2 instances de ./client sur 3 terminaux différents en commençant par ./serveur.

##### Alice
![](https://cdn.discordapp.com/attachments/992077049344819281/1058485784103108648/rxtp4_1.jpg)

##### Bob
![](https://cdn.discordapp.com/attachments/992077049344819281/1058485783910162463/rxtp4_2.jpg)

* **1 :** Les deux clients se connectent au serveur et rentrent leur pseudo (pas d'ordre entre les deux)
* **2 :** Bob affiche sa liste d'abonnements, elle est vide (il vient de se connecter pour la première fois)
* **3 :** Il ajoute Alice à sa liste d'abonnements
* **4 :** On vérifie qu'Alice est bien dans celle-ci
* **5 :** Bob essaye d'ajouter Alice à sa liste mais elle y est déjà
* **6 :** Alice envoie un message à tous ces abonnés, dans le même temps, Bob le reçoit
* **7 :** Après ce message choquant, Bob décide de se désabonner d'Alice
* **8 :** Bob n'a plus d'abonnements.
* **9 :** Alice s'abonne à Bob
* **10 :** Bob envoie un message qu'Alice reçoit
* **11 :** Alice se déconnecte.
* **12 :** Bob envoie un message 
* **13 :** Lorsque Alice se reconnecte, elle est reconnue par le serveur et reçoit le message laissé par Bob (auqeul elle est abonnée) depuis sa dernière déconnexion.
* **14 :** Alice ne se rappelle plus trop des commandes et décide de consulter l'aide.
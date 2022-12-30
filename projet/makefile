
OBJ1 = client.o 
OBJ2 = serveur.o
OBJ3 = types.o  
OBJ4 = serveur_select.o
OPTIONS	=
# Adaptation a Darwin / MacOS X avec fink
# Du fait de l'absence de libtermcap on se fait pas mal
# cracher dessus mais ca marche...
ifeq ($(shell uname), Darwin)
LFLAGS	+= -L/opt/local/lib
CFLAGS	+= -I /opt/local/include
endif
#Changer si necessaire le chemin d'acces aux librairies

# Adaptation a Linux
ifeq ($(shell uname), Linux)
OPTIONS	+= -ltermcap
endif

# Adaptation a FreeBSD
# Attention : il faut utiliser gmake...
ifeq ($(shell uname),FreeBSD)
OPTIONS	+= -ltermcap
endif

# Adaptation a Solaris

ifeq ($(shell uname),SunOS)
OPTIONS	+= -ltermcap  -lsocket -lnsl
CFLAGS	+= -I..
endif

EXEC = ${OBJ1} client ${OBJ2} serveur ${OBJ3} serveur_select ${OBJ4}
all: ${EXEC} 	

#-DDEBUG

types.o : types.c types.h
	gcc -g $(CFLAGS) -c  types.c	

client.o : client.c 
	gcc -g  $(CFLAGS) -c  client.c	

serveur.o : serveur.c 
	gcc -g $(CFLAGS) -c  serveur.c	

serveur_select.o : serveur_select.c 
	gcc -g $(CFLAGS) -c  serveur_select.c	

client : ${OBJ1}	
	gcc -g $(LFLAGS) ${OBJ1} -o client -lcurses   $(OPTIONS)

serveur : ${OBJ2} ${OBJ3}	
	gcc -g $(LFLAGS) ${OBJ2} ${OBJ3} -o serveur -lcurses   $(OPTIONS)

serveur_select : ${OBJ4} ${OBJ3}	
	gcc -g $(LFLAGS) ${OBJ4} ${OBJ3} -o serveur_select -lcurses   $(OPTIONS)

clean : 
	rm -f ${EXEC} core


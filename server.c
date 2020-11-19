/* servTCPConcTh2.c - Exemplu de server TCP concurent care deserveste clientii
   prin crearea unui thread pentru fiecare client.
   Asteapta un numar de la clienti si intoarce clientilor numarul incrementat.
	Intoarce corect identificatorul din program al thread-ului.

   Autor: Lenuta Alboaie  <adria@infoiasi.ro> (c)2009
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include <sqlite3.h>
#include <strings.h>
#include <poll.h>

/* portul folosit */
#define PORT 2908

/* codul de eroare returnat de anumite apeluri */
extern int errno;

int playeri = 0;

typedef struct thData{
	int idThread; //id-ul thread-ului tinut in evidenta de acest program
	int cl; //descriptorul intors de accept
}thData;


static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void raspunde(void *);
void incepe_jocul(struct thData tdL);
int logare(struct thData tdL);
int logare_helper(const char*, const char*);
void incepe_jocul(struct thData tdL);
int nr_random();
void operatii(struct thData tdL);
void trimite_meniu(struct thData tdL);
int executie_joc(struct thData tdL);

int main ()
{
  struct sockaddr_in server;	// structura folosita de server
  struct sockaddr_in from;	
  int nr;		//mesajul primit de trimis la client 
  int sd;		//descriptorul de socket 
  int pid;
  pthread_t th[100];    //Identificatorii thread-urilor care se vor crea
	int i=0;
  

  /* crearea unui socket */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("[server]Eroare la socket().\n");
      return errno;
    }
  /* utilizarea optiunii SO_REUSEADDR */
  int on=1;
  setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
  
  /* pregatirea structurilor de date */
  bzero (&server, sizeof (server));
  bzero (&from, sizeof (from));
  
  /* umplem structura folosita de server */
  /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;	
  /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl (INADDR_ANY);
  /* utilizam un port utilizator */
    server.sin_port = htons (PORT);
  
  /* atasam socketul */
  if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
      perror ("[server]Eroare la bind().\n");
      return errno;
    }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen (sd, 2) == -1)
    {
      perror ("[server]Eroare la listen().\n");
      return errno;
    }
  /* servim in mod concurent clientii...folosind thread-uri */
  while (1)
    {
      int client;
      thData * td; //parametru functia executata de thread     
      int length = sizeof (from);

      printf ("[server]Asteptam la portul %d...\n",PORT);
      fflush (stdout);

      // client= malloc(sizeof(int));
      /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
      if ( (client = accept (sd, (struct sockaddr *) &from, &length)) < 0)
	{
	  perror ("[server]Eroare la accept().\n");
	  continue;
	}
	
        /* s-a realizat conexiunea, se astepta mesajul */
    
	// int idThread; //id-ul threadului
	// int cl; //descriptorul intors de accept

	td=(struct thData*)malloc(sizeof(struct thData));	
	td->idThread=i++;
	td->cl=client;

	pthread_create(&th[i], NULL, &treat, td);	      
				
	}//while    
};				


static void *treat(void * arg)
{		
		struct thData tdL; 
		tdL= *((struct thData*)arg);	
		printf ("[thread]- %d - Asteptam conectarea...\n", tdL.idThread);
		fflush (stdout);		 
		pthread_detach(pthread_self());		
		raspunde((struct thData*)arg);
		/* am terminat cu acest client, inchidem conexiunea */
		close ((intptr_t)arg);
		return(NULL);	
  		
};


void raspunde(void *arg)
{
	struct thData tdL;
	tdL= *((struct thData*)arg);
	int logat = logare(tdL);
	if(logat == 1)
		operatii(tdL);
}

int logare(struct thData tdL) {

    char username_received[128];
    char password_received[128];

    while (1) {

        // cer username-ul clientului
        if (write (tdL.cl, "USERNAME: ", sizeof("USERNAME: ")) <= 0) {
            printf("[Thread %d] ",tdL.idThread);
            perror ("[Thread]Eroare la write() catre client.\n");
        }

        // primesc username-ul clientului
        if (read (tdL.cl, username_received, sizeof(username_received)) <= 0) {
            printf("[Thread %d] ",tdL.idThread);
            perror ("[Thread]Eroare la write() catre client.\n");
        }

        // cer parola clientului
        if (write (tdL.cl, "PASSWORD: ", sizeof("PASSWORD: ")) <= 0) {
            printf("[Thread %d] ",tdL.idThread);
            perror ("[Thread]Eroare la write() catre client.\n");
        }

        // primesc parola clientului
        if (read (tdL.cl, password_received, sizeof(password_received)) <= 0) {
            printf("[Thread %d] ",tdL.idThread);
            perror ("[Thread]Eroare la write() catre client.\n");
        }

        int raspuns = logare_helper(username_received, password_received);
        // trimit mesajul de logare
        if (write (tdL.cl, &raspuns, sizeof(int)) <= 0) {
            printf("[Thread %d] ",tdL.idThread);
            perror ("[Thread]Eroare la write() catre client.\n");
        }

	 if (raspuns == 1) {
        	{printf("Utilizator logat!\n"); fflush(stdout);}
	  	return 1;
	 }

      }

    return 0;
}


int logare_helper(const char* username, const char* password) {
    // ne conectam la baza de date pentru autentificare
    char *err_msg = 0;
    struct sqlite3 *db;
    int rc = sqlite3_open("baza.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }

    // Cautam utilizatorul cu parola
    char SQL[256];
    strcpy(SQL, "SELECT * FROM USERS WHERE ");
    strcat(SQL, "'");
    strcat(SQL, username);
    strcat(SQL, "'");
    strcat(SQL, " = USERS.username AND ");
    strcat(SQL, "'");
    strcat(SQL, password);
    strcat(SQL, "'");
    strcat(SQL, " = USERS.password");


    struct sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, SQL, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1;
    }
}


void incepe_jocul(struct thData tdL){
	printf("Jocul a inceput!\n");
}


int nr_random(){
    srand(time(NULL));
	return rand() % 20;
}


void trimite_meniu(struct thData tdL){
	char menu[4][100];
	strcpy(menu[0],"********[Meniu principal]********");
	strcpy(menu[1],"            1. Joaca             ");
	strcpy(menu[2],"          2.Detalii cont         ");
	strcpy(menu[3],"        3. Paraseste jocul       ");

	if(write(tdL.cl, menu, sizeof(menu)) <= 0){
			printf("[Thread %d] ",tdL.idThread);fflush(stdout);
     		perror ("[Thread]Eroare la write() catre client.\n");
    	}
}


void operatii(struct thData tdL) {

	trimite_meniu(tdL); fflush(stdout);

	while(1) {
		int optiune;
		if(read(tdL.cl, &optiune, sizeof(optiune)) <= 0) {
				printf("[Thread %d] ",tdL.idThread);fflush(stdout);
           		perror ("[Thread]Eroare la write() catre client.\n");
        }
        if(optiune == 1) {
            playeri++ ;
            printf("%d",playeri);
            fflush(stdout);
            
            while (	playeri < 2);
			sleep(1);
			playeri = 0;//jocul a inceput, reinitializam contorul de playeri pentru cei ce se vor conecta ulterior
            printf("Incepem jocul!\n");
            executie_joc(tdL);
            
        }
        else if( optiune == 3)
        	while(optiune==3);
	}
}

struct intrebari{
    int id;
    char intrebare[256];
    char raspuns1[256];
    char raspuns2[256];
    char raspuns3[256];
    char raspuns4[256];
    char raspunsc[64];
}intreb[20];

int idx=0;

int callback(void *NotUsed, int argc, char **argv, char **azColName) {
    
        NotUsed = 0;
    
        intreb[idx].id = atoi(argv[0]);
        strcpy(intreb[idx].intrebare,argv[1]);
        strcpy(intreb[idx].raspuns1,argv[2]);
        strcpy(intreb[idx].raspuns2,argv[3]);
        strcpy(intreb[idx].raspuns3,argv[4]);
        strcpy(intreb[idx].raspuns4,argv[5]);
        strcpy(intreb[idx].raspunsc,argv[6]);
        idx++;
    
        return 0;
}

int memorare_baza_date = 0; //pentru a aduce baza de date o singura data in memorie
int contor_terminare_playeri = 0;
int punctaje[20] = {0};
char d[2][128];

int executie_joc(struct thData tdL) {
    
    int frecv[10] = {0};
    int frecvi = 1;
    
    if( memorare_baza_date == 0) {
    
        sqlite3 *db;
        char *err_msg = 0;
        int rc = sqlite3_open("baza.db", &db);
    
        if (rc != SQLITE_OK) {
            fprintf(stderr, "Cannot open database: %s\n", 
                sqlite3_errmsg(db));
            sqlite3_close(db);
            return 1;
        }
    
        char stmt[64];
    
        strcpy(stmt,"SELECT * FROM INTREBARI");
       
        rc = sqlite3_exec(db, stmt, callback, 0, &err_msg);
    
        if (rc != SQLITE_OK ) {
        
            fprintf(stderr, "Failed to select data\n");
            fprintf(stderr, "SQL error: %s\n", err_msg);

            sqlite3_free(err_msg);
            sqlite3_close(db);
        
            return 1;
        }
    
        sqlite3_close(db);
    
        memorare_baza_date = 1;
    }
    
    int n = nr_random();
    frecv[frecvi++] = n;                                 // pt prima intrebare
    if(write(tdL.cl,&intreb[n],sizeof(intreb[n])) < 0){ //trimit prima intrebare
            printf("[Thread %d] ",tdL.idThread);
     	    perror ("[Thread]Eroare la write() catre client.\n");
    }
    
    for(int k=0;k<9;k++) {
        
        
        int gasit = 0;
        while( gasit == 0 ) {
            
            n=nr_random();
            for(int l=0;l<frecvi;l++)
                if(n == frecv[l])
                    gasit = 1; 

            if( gasit == 1 )
                gasit = 0;
            else {
                frecv[frecvi++] = n;
                gasit = 1;
            }
        }
        char optiune;//raspunsul trimis de client
        
        
                              //daca am primit confirmarea ca au raspuns toti playerii, trimit urmatoarea intrebare
                              
        
        if(write(tdL.cl,&intreb[n],sizeof(intreb[n])) < 0){
            printf("[Thread %d] ",tdL.idThread);
     	    perror ("[Thread]Eroare la write() catre client.\n");
     	    }
     	    	
    }
    
	
	
    int nrpuncte;
    int max = 0;
    int idmax = 0;
    int eroare[20] = {0};
    char c[128];
    
    
    if(read(tdL.cl,&nrpuncte,sizeof(nrpuncte)) <= 0){//punctaj
    			printf("[Thread %d] ", tdL.idThread);
			    perror ("[Thread]Eroare la read() de la client.\n");
            	return errno;
    }
    
    if(read(tdL.cl,&c,sizeof(c)) <= 0){//nume
			    printf("[Thread %d] ", tdL.idThread);
			    perror ("[Thread]Eroare la read() de la client.\n");
            	return errno;
            	eroare[tdL.idThread] = 1;
    }
    
    contor_terminare_playeri++;
    punctaje[tdL.idThread] = nrpuncte;
    
    strcpy(d[tdL.idThread],c);
    
    while(contor_terminare_playeri != 2);
    
    if(contor_terminare_playeri == 2)
    	for(int i=0;i<2;i++)
    	{	
    		if(punctaje[i] > max) {
    			max = punctaje[i];
    			idmax = i;
    		}
    	}
    	
    if(contor_terminare_playeri == 2) {
    	
    	if(eroare[tdL.idThread] == 0)//adica daca nu a parasit jocul deodata
    		 if(write(tdL.cl,&idmax,sizeof(idmax)) <= 0){
            		printf("[Thread %d] ",tdL.idThread);fflush(stdout);
     	   			perror ("[Thread]Eroare la write() catre client.\n");
     	    }
     	
    }
    	printf("CASTIGATOR %s\n",d[idmax]); fflush(stdout);
    	sleep(1);
     	contor_terminare_playeri = 0;
 
}

/* cliTCPIt.c - Exemplu de client TCP
   Trimite un numar la server; primeste de la server numarul incrementat.
         
   Autor: Lenuta Alboaie  <adria@infoiasi.ro> (c)2009
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <poll.h>

#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_RESET   "\x1b[0m"
#define COLOR_GRAY    "\x1b[37m"

/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;

struct intrebari{
    int id;
    char intrebare[256];
    char raspuns1[256];
    char raspuns2[256];
    char raspuns3[256];
    char raspuns4[256];
    char raspunsc[64];
};

int main (int argc, char *argv[])
{
  int sd;			// descriptorul de socket
  struct sockaddr_in server;	// structura folosita pentru conectare 
  		// mesajul trimis
  int nr=0;
  char buf[10];
  struct pollfd mypoll = { STDIN_FILENO, POLLIN|POLLPRI };
  
  /* exista toate argumentele in linia de comanda? */
  if (argc != 3)
    {
      printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
      return -1;
    }

  /* stabilim portul */
  port = atoi (argv[2]);

  /* cream socketul */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("Eroare la socket().\n");
      return errno;
    }

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr(argv[1]);
  /* portul de conectare */
  server.sin_port = htons (port);

  /* ne conectam la server */
  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
      perror ("[client]Eroare la connect().\n");
      return errno;
    }

    char username[128];
    char parola[128];
    char message[64];

    /* citirea raspunsului dat de server 
    (apel blocant pana cand serverul raspunde) */

    while (1) {
        // primim cererea de username de la server
        if (read(sd, message, sizeof(message)) < 0) {
            perror ("[client]Eroare la read() de la server.\n");
            return errno;
        }

        printf("%s", message);fflush(stdout);
        scanf("%s", username);
        // trimitem username-ul serverului
        if (write(sd, username,sizeof(username)) <= 0) {
            perror("[client]Eroare la write() spre server.\n");
            return errno;
        }

        // primim cererea de parola de la server
        if (read(sd, message, sizeof(message)) < 0) {
            perror ("[client]Eroare la read() de la server.\n");
            return errno;
        }

        printf("%s", message);fflush(stdout);
        scanf("%s", parola);

        // trimitem parola serverului
        if (write(sd, parola, sizeof(parola)) <= 0) {
            perror("[client]Eroare la write() spre server.\n");
            return errno;
        }

        // primim mesajul de status logare
        int logare_status;
        if (read(sd, &logare_status, sizeof(int)) < 0) {
            perror ("[client]Eroare la read() de la server.\n");
            return errno;
        }

        if (logare_status == 1) {
            system("clear");
            printf(COLOR_GREEN "BINE AI VENIT, %s!\n" COLOR_RESET, username);
            fflush(stdout);
	        break;
        }
        else {
            printf(COLOR_RED "USERNAME SAU PAROLA GRESITE, INCERCATI DIN NOU!\n" COLOR_RESET);
            fflush(stdout);
        }
   }

    char menu[4][100];
    	if(read(sd, menu, sizeof(menu)) < 0){
    		perror ("[client]Eroare la read() de la server.\n");
        	return errno;
    	}	

	while(1) {

	for(int i=0; i<4; i++){
	    if( i == 0)
	        {printf( COLOR_BLUE "%s \n" COLOR_RESET, menu[i] ); fflush(stdout);}
		else
		    {printf( COLOR_YELLOW "%s\n" COLOR_RESET, menu[i]); fflush(stdout);}
        }
	
		int optiune;
		printf("Introduceti optiunea dorita: "); fflush(stdout);
		scanf("%d", &optiune);

		if(write(sd, &optiune, sizeof(optiune)) < 0) {
					perror ("[client]Eroare la read() de la server.\n");
            		return errno;
		}

        struct intrebari intrebare;

		if(optiune == 1) {
			printf("Asteptam conectarea celorlaltor jucatori.\n");fflush(stdout);
			int puncte = 0;
			for(int k=1;k<=10;k++) {
			
			if(read(sd,&intrebare,sizeof(intrebare)) < 0){
			    perror ("[client]Eroare la read() de la server.\n");
            	return errno;
            }
            
            system("clear");
            
            printf(COLOR_MAGENTA "%s \n" COLOR_RESET ,intrebare.intrebare);
            fflush(stdout);
            printf(COLOR_CYAN "%s\n" COLOR_RESET,intrebare.raspuns1);	
            fflush(stdout);
            printf(COLOR_CYAN "%s\n" COLOR_RESET,intrebare.raspuns2);	
            fflush(stdout);
            printf(COLOR_CYAN "%s\n" COLOR_RESET,intrebare.raspuns3);	
            fflush(stdout);
            printf(COLOR_CYAN "%s\n" COLOR_RESET,intrebare.raspuns4);
            fflush(stdout);
            
            char rasp[10];
            char tmp;
            char raspuns_final[64];
            
            printf(COLOR_GRAY "Aveti 10 secunde la dispozitie sa raspundeti la intrebare.\n" COLOR_RESET);
            fflush(stdout);
            
            if( poll(&mypoll, 1, 10000) )  //timer de 10 secunde
            {
                scanf("%c", &tmp);//pt a copia enter
                scanf("%s", rasp);
            }
            else
                {printf(COLOR_RED "Timpul a expirat.\n" COLOR_RESET);fflush(stdout);}
            
            strcpy(raspuns_final,rasp);
            
            while(rasp[0]!='a' && rasp[0]!='b' && rasp[0]!='c' && rasp[0]!='d' && rasp[0]!='\0')
                {
                printf("Raspuns formulat gresit. Introduceti doar o litera:\n");
                fflush(stdout);
                scanf("%s", rasp);
                strcpy(raspuns_final,rasp);
                }
                
            
            if(raspuns_final[0]==intrebare.raspunsc[0] && raspuns_final[0]!='\0'){
			    printf(COLOR_GREEN "Raspuns corect!\n" COLOR_RESET);
			    fflush(stdout);
			    puncte++;
			    sleep(1);
            	}
            else {
                printf(COLOR_RED "RASPUNS GRESIT\n" COLOR_RESET);
                fflush(stdout);
                sleep(1);
                }
                
            raspuns_final[0]='\0';//golim sirul de raspuns
            rasp[0]='\0';
                
            
           }//for de 10 intrebari
           
           if(write(sd,&puncte,sizeof(puncte)) <= 0){
			    perror ("[client]Eroare la write() catre server.\n");
            	return errno;
            }
            
            if(write(sd,username,sizeof(username)) <= 0){
			    perror ("[client]Eroare la write() catre server.\n");
            	return errno;
            }
            
            printf("Asteptam sa termine toti playerii pentru a anunta castigatorul.\n"); fflush(stdout);
            char winner[128];
            
            if(read(sd,winner,sizeof(winner)) < 0){
			    perror ("[client]Eroare la read() de la server.\n");
            	return errno;
            }
            
            printf("%s \n", winner); fflush(stdout);
            sleep(10);
           
         }//optiunea 1	
         
		 else if (optiune == 2) {
		            system("clear");
		            printf("Aceste informatii va sunt afisate pentru cateva secunde, apoi veti fi redirectionat automat catre meniul principal.\n");fflush(stdout);
            		printf(COLOR_GREEN "Nume utilizator: " COLOR_RESET); printf("%s \n", username);fflush(stdout);
            		printf(COLOR_GREEN "Parola: " COLOR_RESET); printf("%s \n", parola);fflush(stdout);
            		sleep(7);
         }
         else if (optiune == 3)	{
                    printf("Ati fost deconectat.\n");fflush(stdout);
                    return 0;
            		break;
         }
         system("clear");
	}


  /* inchidem conexiunea, am terminat */
  close (sd);
}


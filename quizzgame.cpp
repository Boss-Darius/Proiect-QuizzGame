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
#include <semaphore.h>
#include"rapidxml.hpp"
#include"rapidxml_print.hpp"
#include"rapidxml_utils.hpp"
#include"SDL.h"
#include"SDL_mixer.h"
#include"SDL_ttf.h"
using namespace rapidxml; //pentru manipularea fisierului xml



#define PORT 2908



typedef struct thData{
	int idThread; //id-ul thread-ului tinut in evidenta de acest program
	int cl; //descriptorul intors de accept
}thData;

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void raspunde(void *);

int numar_jucatori=0;




void parcurgere_vecini(xml_node<>* nod)
{
  xml_node<>* vecin=nod;

  while(vecin)
  {
     xml_attribute<>* atribut=vecin->first_attribute();
     while(atribut)
     {
      printf("%s\n",atribut->value());
      atribut=atribut->next_attribute();
     }    

    vecin=vecin->next_sibling();
  }


}
sem_t semafor;
int alegeri[4];

int getmax()
{
  int i,max=0;

  for(i=0;i<4;i++)
  {
    if(alegeri[i]>=max) max=alegeri[i];
  }

  for(i=3;i>0;i++)
  {
    if(alegeri[i]==max) break;

  }

  return i;
}
time_t cronometru=time(NULL)+10;
int generator;

int scor_clienti[100];
char nume_clienti[100][256];

int castigator()
{
  int pozitie;
  int max=0;
  for(pozitie=0;pozitie<numar_jucatori;pozitie++)
  {
    if(max<=scor_clienti[pozitie]) max=scor_clienti[pozitie];
  }

  return max;
}

pthread_t th[100];    //Identificatorii thread-urilor care se vor crea


int scor_maxim;
int numar_castigator;

int main ()
{ 
      if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0 )
    {
        printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
        //success = false;
    }
    int init=Mix_Init(0);
                   if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 )
                {
                    printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
                    //success = false;
                }
    Mix_Music* melodie=Mix_LoadMUS("theme 2.wav");
    if(melodie==0) printf("eroare la muzicuta\n");

    Mix_PlayMusic(melodie,-1);
  
  
  
  srand(time(0));
  generator=(rand())%4;
  sem_init(&semafor,0,1);



  file<> quizzes("quizzes.xml"); // Default template is char
  xml_document<> doc;
  doc.parse<0>(quizzes.data());
  xml_node<>* quiz=doc.first_node()->first_node();
  //parcurgere_vecini(quiz);//prezentarea intrebarilor

  struct sockaddr_in server;	// structura folosita de server
  struct sockaddr_in from;	
  int nr;		//mesajul primit de trimis la client 
  int sd;		//descriptorul de socket 
  int pid;
	//int i=0;
  

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


        printf("quizzuri disponibile:\n");
        parcurgere_vecini(quiz);
  while (1)
    {
      int client;
      thData * td; //parametru functia executata de thread     
      socklen_t length = sizeof (from);

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
	td->idThread=numar_jucatori++;
	td->cl=client;

	pthread_create(&th[numar_jucatori], NULL, &treat, td);	      
	}//while    
};				
static void *treat(void * arg)
{	  printf("treat\n"); 
  int id=numar_jucatori;
   //pthread_detach(pthread_self());	
		struct thData tdL; 
		tdL= *((struct thData*)arg);

  char nume[256];
  printf("nume:\n");
  bzero(nume,256);
  write(tdL.cl,"nume:",256);
  read(tdL.cl,nume,256);

  printf("numele dumneavoastra:%s\n",nume);
  strcpy(nume_clienti[id],nume);
  printf("%s",nume_clienti[id]);

  printf("%d\n",id);

		fflush (stdout);		 		
		raspunde((struct thData*)arg);
    printf("calculam rezultatele %d\n",numar_jucatori);

    if(id!=numar_jucatori) pthread_join(th[id+1],NULL);

    printf("numarul castigatorului %d\n",numar_castigator);
    printf("castigatorul este: %s\n",nume_clienti[numar_castigator]);


    char rezultat[256];
    bzero(rezultat,256);
    strcat(rezultat,"Castigatorii: ");


    int max=0;
    for(int i=1;i<=numar_castigator;i++)
    {
        if(scor_clienti[i]>=max) max=scor_clienti[i];
    }

    for(int i=1;i<=numar_jucatori;i++)
    {
      if(scor_clienti[i]==max)
      {
        strcat(rezultat,nume_clienti[i]);
        strcat(rezultat," ");
      }
    }
    strcat(rezultat,"\n");
    write(tdL.cl,rezultat,256);
		/* am terminat cu acest client, inchidem conexiunea */
		close ((intptr_t)arg);


    if(id==1)
    {
      for(int i=2;i<=numar_jucatori;i++)
      {
        pthread_join(th[i],NULL);
      }
      for(int i=1;i<=numar_jucatori;i++)
      {
        scor_clienti[i]=0;
        strcpy(nume_clienti[i],"0");
      }

      numar_jucatori=0;
    }
		return(NULL);	
  		
  		
};
//time_t 

void raspunde(void *arg)
{
  int id=numar_jucatori;
  int scor=0;
  char mesaj[256];
  char raspuns[256];
  char titlu[256];
  strcpy(titlu,"Intrebari pentru ");
  strcat(titlu,nume_clienti[id]);

	struct thData tdL; 
	tdL= *((struct thData*)arg);

  int conectat=1;

  file<> quizzes("quizzes.xml"); // Default template is char
  xml_document<> doc;
  doc.parse<0>(quizzes.data());
  xml_node<>* quiz=doc.first_node()->first_node();

  for(int i=0; i<generator;i++)
  {
    quiz=quiz->next_sibling();
  }

 
  SDL_Window* fereastra=SDL_CreateWindow(titlu,SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,500,500,SDL_WINDOW_SHOWN);
  SDL_Renderer* spectru=SDL_CreateRenderer(fereastra,-1,0);
  SDL_SetRenderDrawColor(spectru,0,0,0,255);
  SDL_RenderClear(spectru);
  SDL_RenderPresent(spectru);


  TTF_Init();
  TTF_Font * font = TTF_OpenFont("OpenSans-Bold.ttf", 25);

  SDL_Color alb={255,255,255};

  SDL_Surface *suprafata;

  int texW = 0;
  int texH = 0;
  SDL_Texture * texture = SDL_CreateTextureFromSurface(spectru, suprafata);


  char afisare[2560];
  int numar_intrebare=1;
  xml_node<>* intrebare=quiz->first_node();
  {
    while(intrebare!=0)
    {
      SDL_RenderClear(spectru);
      printf("jucatorul numarul %d trebuie sa astepte %d secunde \n",id,(id-1)*5);

      for(int i=1;i<=(id-1)*2;i++)
      {
        sleep(1);
      }

      bzero(mesaj,256);
      bzero(raspuns,256);
      bzero(afisare,2560);

      xml_node<>*enunt=intrebare->first_node();
      printf("%d)%s\n",numar_intrebare,enunt->first_attribute()->value());
      strcat(afisare,enunt->first_attribute()->value());
      strcat(afisare,"\n");
      xml_node<>* posibilitate=enunt->next_sibling();
      while(posibilitate)
      {
        printf("%s\n",posibilitate->last_attribute()->value());
        strcat(afisare,posibilitate->last_attribute()->value());
        strcat(afisare,"\n");
        posibilitate=posibilitate->next_sibling();
      }

      xml_node<>*optiunea_ta=enunt->next_sibling();;

        suprafata= TTF_RenderText_Blended_Wrapped(font,afisare,alb,320);
        SDL_Texture * texture = SDL_CreateTextureFromSurface(spectru, suprafata);
        SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);
        SDL_Rect dstrect = { 0, 0, texW, texH };
        SDL_RenderCopy(spectru, texture, NULL, &dstrect);
        SDL_RenderPresent(spectru);

        SDL_FreeSurface(suprafata);

      read(tdL.cl,mesaj,256);

      if(strcmp(mesaj,"a\n")==0)
      {
          strcpy(raspuns,optiunea_ta->first_attribute()->value());
          if(strcmp(raspuns,"corect")==0)
          {
            scor++;
            sem_wait(&semafor);
            if(scor>=scor_maxim)
            {
              scor_maxim=scor;
              numar_castigator=id;
            }
            sem_post(&semafor);
          }
      }
      if(strcmp(mesaj,"b\n")==0)
      {
          strcpy(raspuns,optiunea_ta->next_sibling()->first_attribute()->value());
          if(strcmp(raspuns,"corect")==0)
          {
            scor++;
            sem_wait(&semafor);
            if(scor>=scor_maxim)
            {
              scor_maxim=scor;
              numar_castigator=id;
            }
            sem_post(&semafor);
          }
      }
      if(strcmp(mesaj,"c\n")==0)
      {
          strcpy(raspuns,optiunea_ta->next_sibling()->next_sibling()->first_attribute()->value());
          if(strcmp(raspuns,"corect")==0)
          {
            scor++;
           sem_wait(&semafor);
            if(scor>=scor_maxim)
            {
              scor_maxim=scor;
              numar_castigator=id;
            }
            sem_post(&semafor);
          }          
      }
      if(strcmp(mesaj,"d\n")==0)
      {
          strcpy(raspuns,optiunea_ta->next_sibling()->next_sibling()->next_sibling()->first_attribute()->value());
          if(strcmp(raspuns,"corect")==0)
          {
            scor++;
           sem_wait(&semafor);
            if(scor>=scor_maxim)
            {
              scor_maxim=scor;
              numar_castigator=id;
            }
            sem_post(&semafor);
          }
      }

      write(tdL.cl,raspuns,256);
      numar_intrebare++;
      intrebare=intrebare->next_sibling();
      SDL_RenderClear(spectru);
    }
    sem_wait(&semafor);
    printf("scorul tau:%d\n",scor);
    scor_clienti[id]=scor;
    SDL_DestroyWindow(fereastra);
    sem_post(&semafor);

  }

}
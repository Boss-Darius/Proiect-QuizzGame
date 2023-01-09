#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>

int main (int argc, char *argv[])
{
    int jucator;        //descriptorul de socket pentru utilizator
    char comanda[256];   //comanda data de utilizator
    struct sockaddr_in quiz;

    if (argc!=3)
    {
        printf("sintaxa gresita \n");
        return -1;
    }

    int port = atoi (argv[2]); //stabilim portul

    jucator=socket(AF_INET,SOCK_STREAM,0);

    if(jucator<0)
    {
        printf("eroare la crearea socket-ului");
        return -2;
    }

    quiz.sin_family = AF_INET;
    quiz.sin_addr.s_addr = inet_addr(argv[1]);
    quiz.sin_port = htons (port);

    int conexiune=connect(jucator,(struct sockaddr*) &quiz,sizeof (struct sockaddr));

    if(conexiune<0)
    {
        printf("eroare la conexiune \n");
        return -3;
    }
    int conectat=1;

    while(conectat)
    {
        bzero(comanda,256);
        fgets(comanda,256,stdin);

        if(strcmp(comanda,"quit\n")==0)
        {
            printf("Ati parasit jocul :'( \n");
            conectat=0;
        }
        //fgets(comanda,256,stdin);


        if(send(jucator,comanda,256,0)<0)
        {
            printf("eroare la trimiterea comenzii catre server \n");
            return-4;
        }
        bzero(comanda,256);
        if(recv(jucator,comanda,256,0)<0)
        {
            printf("eroare la primirea raspunsului de la server \n");
            return -5;
        }
        printf("mesajul primit: \n");
        printf("%s",comanda);
    }

    close(jucator);
}
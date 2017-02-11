/******************************************************************************
* echo_server.c                                                               *
*                                                                             *
* Description: This file contains the C source code for an echo server.  The  *
*              server runs on a hard-coded port and simply write back anything*
*              sent to it by connected clients.  It does not support          *
*              concurrent clients.                                            *
*                                                                             *
* Authors: Athula Balachandran <abalacha@cs.cmu.edu>,                         *
*          Wolf Richter <wolf@cs.cmu.edu>                                     *
*                                                                             *
*******************************************************************************/

#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/select.h>
#define ECHO_PORT 9999
#define BUF_SIZE 4096
#define MAXN 512
int close_socket(int sock)
{
    if (close(sock))
    {
        fprintf(stderr, "Failed closing socket.\n");
        return 1;
    }
    return 0;
}

int main(int argc, char* argv[])
{
    int master_sock, client_sock[MAXN], new_sock, n;
    ssize_t readret;
    fd_set readfds;
    //struct timeval tv;
    socklen_t cli_size;
    struct sockaddr_in addr, cli_addr;
    char buf[BUF_SIZE];

    fprintf(stdout, "----- Echo Server -----\n"); 

    for (int i = 0; i < MAXN; i ++){
        client_sock[i] = 0;
    }

    //fprintf(stdout, "----- Echo Server -----\n"); 
    /* all networked programs must create a socket */
    if ((master_sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        fprintf(stderr, "Failed creating socket.\n");
        return EXIT_FAILURE;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(ECHO_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    /* servers bind sockets to ports---notify the OS they accept connections */
    if (bind(master_sock, (struct sockaddr *) &addr, sizeof(addr)))
    {
        close_socket(master_sock);
        fprintf(stderr, "Failed binding socket.\n");
        return EXIT_FAILURE;
    }


    //fprintf(stdout, "----- Echo Server -----\n"); 
    if (listen(master_sock, 5))
    {
        close_socket(master_sock);
        fprintf(stderr, "Error listening on socket.\n");
        return EXIT_FAILURE;
    }
    //fprintf(stdout,"tes");
    //tv.tv_sec = 0;
    //tv.tv_usec = 0;
    /* finally, loop waiting for input and then write it back */
    while (1)
    {
       
       FD_ZERO(&readfds);
       FD_SET(master_sock, &readfds);
       n = master_sock;
       
       for (int i = 0; i < MAXN; i ++){
           if (client_sock[i] > 0)
               FD_SET(client_sock[i], &readfds);
           if (client_sock[i] > n)
               n = client_sock[i];
       }
       if (select(n + 1, &readfds, NULL, NULL, NULL) == -1){
           fprintf(stderr, "Select error!\n");
           return EXIT_FAILURE;
       }
       
       if (FD_ISSET(master_sock, &readfds)){
           if ((new_sock = accept(master_sock, (struct sockaddr *) &cli_addr, &cli_size)) == -1)
           {
               close(master_sock);
               fprintf(stderr, "Error accepting connection.\n");
               return EXIT_FAILURE;
           }
           fprintf(stdout,"New connection.\n");
           
           
           for (int i = 0; i < MAXN; i++){
               if (client_sock[i] == 0){
                   client_sock[i] = new_sock;
                   printf("%d\n",i);
                   break;
               }
           }
       }
       printf("fuck\n");
       for (int i = 0; i < MAXN; i++){
           int sd = client_sock[i];

           if (FD_ISSET(sd, &readfds)){
               printf("receiving from %d\n", i);
               cli_size = sizeof(cli_addr);

               readret = 0;

               if ((readret = recv(sd, buf, BUF_SIZE, 0)) >= 1)
               {
                   if (send(sd, buf, readret, 0) != readret)
                   {
                       close_socket(sd);
                       close_socket(master_sock);
                       fprintf(stderr, "Error sending to client.\n");
                       return EXIT_FAILURE;
                   }
                   memset(buf, 0, BUF_SIZE);
                   printf("%d th read %d bytes\n",i,(int)readret);
               } 
               //fprintf(stdout,"%d end reading\n", i);
               if (readret == -1)
               {
                   close_socket(sd);
                   close_socket(master_sock);
                   fprintf(stderr, "Error reading from client socket.\n");
                   return EXIT_FAILURE;
               }
               
               if (readret == 0){
                   if (close_socket(sd))
                   {
                       close_socket(master_sock);
                       fprintf(stderr, "Error closing client socket.\n");
                       return EXIT_FAILURE;
                   }
                   client_sock[i] = 0;
              }
            }
        }
    }
    close_socket(master_sock);
    return EXIT_SUCCESS;
}

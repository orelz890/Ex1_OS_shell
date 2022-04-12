
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/select.h>
#include <string.h>

#define PORT 6060
#define IP "127.0.0.1"
#define ERROR -1
#define SUCCESS 0
#define BUFFER_SIZE 1024
#define MAXMSG  1024

// Was aided by this site: https://www.gnu.org/software/libc/manual/html_node/Server-Example.html

struct sockaddr_in server_address;
int server_socket;


void set_socket(){

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == ERROR){
        perror("Error in set_socket - socket error\n");
        exit(EXIT_FAILURE);
    }
    int optval = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == ERROR){
        perror("Error in set_socket - setsockopt error\n");
        exit(EXIT_FAILURE);
    }
    int binded = bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address));
    if (binded == ERROR){
        perror("Error in set_socket - bind failed\n");
        exit(EXIT_FAILURE);
    }
    // Max queue length is 5 if the queue is full,
    // the client may receive an error with an indication of ECONNREFUSED
    if (listen(server_socket, 5) == ERROR){
        perror("Error in set_socket - listen failed\n");
        exit(EXIT_FAILURE);
    }
    printf("Listening\n");
}

int read_from_client (int filedes){
    
    char buffer[MAXMSG];
    memset(buffer,0,sizeof(buffer));
    int nbytes;

    nbytes = read (filedes, buffer, MAXMSG);
    if (nbytes < 0)
    {
        /* Read error. */
        perror ("read");
        exit (EXIT_FAILURE);
    }
    else if (nbytes == 0)
    /* End-of-file. */
    return -1;
    else{
        /* Data read. */
        fprintf (stderr, "Server: got message: %s\n", buffer);
        return 0;
    }
}

int main (void){

    set_socket();

    fd_set active_fd_set, read_fd_set;
    struct sockaddr_in clientname;

    /* Initialize the set of active sockets. */
    FD_ZERO (&active_fd_set);
    FD_SET (server_socket, &active_fd_set);

    while (1){
        /* Block until input arrives on one or more active sockets. */
        read_fd_set = active_fd_set;
        if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0){
            perror ("ERROR - select");
            exit (EXIT_FAILURE);
        }

        /* Service all the sockets with input pending. */
        for (int i = 0; i < FD_SETSIZE; ++i)
            if (FD_ISSET (i, &read_fd_set)){
                if (i == server_socket){
                    /* Connection request on original socket. */
                    socklen_t size = sizeof (clientname);
                    int new = accept (server_socket,(struct sockaddr *) &clientname,&size);
                    if (new < 0){
                        perror ("ERROR - accept");
                        exit (EXIT_FAILURE);
                    }
                    printf("New client connected\n");
                    FD_SET (new, &active_fd_set);
                }
                else{
                    /* Data arriving on an already-connected socket. */
                    if (read_from_client (i) < 0){
                        close (i);
                        FD_CLR (i, &active_fd_set);
                    }
                }
            }
    }
}

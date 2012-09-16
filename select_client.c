#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{

    fd_set fd_list;    // fd_list file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number
    int nbytes;

    int socket_to_server_fd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    char key_buffer;

    FD_ZERO(&fd_list);    // clear the fd_list and temp sets
    FD_ZERO(&read_fds);

    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    socket_to_server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_to_server_fd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(socket_to_server_fd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        error("ERROR connecting");
    }

    FD_SET(STDIN_FILENO, &fd_list);
    FD_SET(socket_to_server_fd, &fd_list);

    // keep track of the biggest file descriptor
    fdmax = socket_to_server_fd; // so far, it's this one

    for(;;) {
        read_fds = fd_list; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }

        // this is ugly, it should build a buffer not send a byte at a time.
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            key_buffer = fgetc(stdin);

            bzero(buffer,256);
            buffer[0] = key_buffer;
            write(socket_to_server_fd,buffer,strlen(buffer));
        }

        // run through the existing connections looking for data to read
        if (FD_ISSET(socket_to_server_fd, &read_fds)) { // we got one!!
            // handle data from a client
            bzero(buffer,256);
            nbytes = recv(socket_to_server_fd, buffer, sizeof buffer, 0);
            printf("MSG: %s\n", buffer);
        } // END handle data from client
    } // END for(;;)--and you thought it would never end!

    /*
    for(;;) {
      printf("Please enter the message: ");
      bzero(buffer,256);
      fgets(buffer,255,stdin);
      n = write(socket_to_server_fd,buffer,strlen(buffer));
      if (n < 0) 
           error("ERROR writing to socket");
      bzero(buffer,256);

      n = read(socket_to_server_fd,buffer,255);
      printf("%s\n",buffer);
      //while(n > 0) {
      //  printf("%s\n",buffer);
      //  n = read(socket_to_server_fd,buffer,255);
      //}
    }*/

    //if (n < 0) 
    //     error("ERROR reading from socket");
    //printf("%s\n",buffer);
    close(socket_to_server_fd);
    return 0;
}

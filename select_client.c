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

    fd_set fd_list;  // fd_list file descriptor list
    fd_set read_fds; // temp file descriptor list for select()
    int fdmax;       // maximum file descriptor number

    int socket_to_server_fd;
    int portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    char key_buffer;

    FD_ZERO(&fd_list);
    FD_ZERO(&read_fds);

    if (argc < 3) {
        fprintf(stderr,"usage %s hostname port\n", argv[0]);
        exit(0);
    }

    socket_to_server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_to_server_fd < 0) {
        error("ERROR opening socket");
    }

    server = gethostbyname(argv[1]);
    portno = atoi(argv[2]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    if (connect(socket_to_server_fd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR connecting");
    }

    FD_SET(STDIN_FILENO, &fd_list);
    FD_SET(socket_to_server_fd, &fd_list);

    fdmax = socket_to_server_fd;

    for(;;) {
        read_fds = fd_list;
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }

        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            bzero(buffer,256);
            fgets(buffer,255,stdin);
            write(socket_to_server_fd,buffer,strlen(buffer));
        }

        if (FD_ISSET(socket_to_server_fd, &read_fds)) {
            bzero(buffer,256);
            recv(socket_to_server_fd, buffer, sizeof buffer, 0);
            printf("MSG: %s\n", buffer);
        }
    }

    close(socket_to_server_fd);
    return 0;
}

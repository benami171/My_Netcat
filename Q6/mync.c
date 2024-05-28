#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <netdb.h>
#include <poll.h>
#include <sys/select.h>
#include <sys/un.h>
#include <netdb.h>

void RUN(char *args_as_string)
{
    // tokenize the string - split by space
    char *token = strtok(args_as_string, " ");

    if (token == NULL)
    {
        fprintf(stderr, "No arguments provided\n");
        exit(1);
    }
    // create an array of strings to store the arguments
    char **args = (char **)malloc(sizeof(char *));
    int n = 0;         // number of arguments
    args[n++] = token; // add the first argument

    // get the rest of the arguments
    while (token != NULL)
    {
        token = strtok(NULL, " ");                               // get the next token (NULL - take the next token from the previous string)
        args = (char **)realloc(args, (n + 1) * sizeof(char *)); // allocate memory for the new argument
        args[n++] = token;                                       // add the new argument and increment the number of arguments
    }

    // fork and execute the program
    int fd = fork();
    if (fd < 0)
    { // fork failed
        fprintf(stderr, "Fork failed\n");
        exit(1);
    }

    if (fd == 0)
    { // child process
        execvp(args[0], args);
        fprintf(stderr, "Exec failed\n");
        free(args);
        exit(1);
    }
    else
    {
        wait(NULL); // wait for the child process to finish
        // free the memory
        free(args);
        fflush(stdout);
    }
}

void sockets_terminator(int *descriptors)
{
    if (descriptors[0] != STDIN_FILENO)
    {
        close(descriptors[0]);
    }
    if (descriptors[1] != STDOUT_FILENO)
    {
        close(descriptors[1]);
    }
}

void handle_alarm(int sig)
{

    // Terminate the process
    exit(0);
}

// sending the descriptor to handel, and the portnumber to open the server on
void TCP_SERVER(int *descriptors, int port, char *b_flag)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    printf("TCP socket created\n");

    // allow the socket to be reused, maybe to change to fork?
    int optval = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // listen to any address

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, 1) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // maybe to open here fork?
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    // chanigng the input_fd to the new socket after accepting the connection
    int client_fd = accept(sockfd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    descriptors[0] = client_fd;
    if (b_flag != NULL)
    {
        descriptors[1] = client_fd;
    }
}

void TCP_client(int *descriptors, char *ip, int port)
{
    // open a TCP client to the server
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        perror("error creating socket");
        exit(1);
    }

    printf("TCP client\n");
    fflush(stdout);

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
    {
        perror("setsockopt");
        exit(1);
    }

    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0)
    {
        perror("Invalid address/ Address not supported");
        exit(1);
    }

    // connecting to the server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("error connecting to server");
        if (descriptors[0] != STDIN_FILENO) // to ensure we'rent getting input from another place
        {
            close(descriptors[0]);
        }
        exit(1);
    }

    descriptors[1] = sock; // changing the output to form the socket to the client
}

void UDP_SERVER(int *descriptors, int port, int timeout)
{
    // open a UDP server to listen to the port
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1)
    {
        perror("error creating socket");
        sockets_terminator(descriptors);
        exit(1);
    }
    printf("UDP Socket created\n");

    // if not set, the port will be in use for 2 minutes after the program ends
    int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    {
        perror("setsockopt failed");
        sockets_terminator(descriptors);
        exit(1);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("error binding socket");
        sockets_terminator(descriptors);
        exit(1);
    }

    // read the data from the client
    char buffer[1024];
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    int numbytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &client_addr_len);
    if (numbytes == -1)
    {
        perror("error receiving data");
        sockets_terminator(descriptors);
        exit(1);
    }

    if (connect(sockfd, (struct sockaddr *)&client_addr, sizeof(client_addr)) == -1)
    {
        perror("error connecting to client");
        sockets_terminator(descriptors);
        exit(1);
    }

    if (sendto(sockfd, "ACK", 3, 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("error sending ACK");
        exit(1);
    }

    descriptors[0] = sockfd; // changing the descriptor to be the socket
    alarm(timeout);
}

void UDP_CLIENT(int *descriptors, char *ip, int port)
{

    // open a UDP client to the server
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1)
    {
        perror("error creating socket");
        exit(1);
    }
    printf("UDP client\n");
    fflush(stdout);

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0)
    {
        perror("Invalid address/ Address not supported");
        exit(1);
    }

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("error creating socket");
        exit(EXIT_FAILURE);
    }
    printf(" the server ip is %s\n", ip);
    printf(" the server port is %d\n", port);
    // sendto(sockfd, "Starting game\n", 14, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    // "connect" to the server - so if we use sendto/recvfrom, we don't need to specify the server address
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("error connecting to server");
        exit(1);
    }

    descriptors[1] = sockfd; // changing the output to be the socket
}

// UNIX Domain Socket
void UDS_SERVER_STREAM(char *path, int *descriptors)
{
    // opening a UDS server
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0); // for stream like tcp protocol
    if (sockfd == -1)
    {
        sockets_terminator(descriptors);
        perror("error creating socket");
        exit(1);
    }

    printf("1.UDS socket created\n");
    struct sockaddr_un server_addr;     // creating the server address
    server_addr.sun_family = AF_UNIX;   // setting the family to be UNIX
    strcpy(server_addr.sun_path, path); // setting the path to be the path we got from the input

    // binding the socket to the server address
    unlink(path); // remove the file if it exists
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("error binding socket");
        sockets_terminator(descriptors);
        exit(1);
    }

    printf("2.UDS binded\n");

    // listening to incoming connections, we set it to be 1 connection at most.
    if (listen(sockfd, 1) == -1)
    {
        perror("error listening");
        sockets_terminator(descriptors);
        exit(1);
    }

    printf("3.UDS listening\n");

    // creating client struct to store the client address and accepting the connection
    struct sockaddr_un client_addr;
    socklen_t client_len = sizeof(client_addr);

    int client_fd = accept(sockfd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd == -1)
    {
        perror("error accepting connection");
        sockets_terminator(descriptors);
        exit(1);
    }

    printf("4.UDS accepted\n");

    descriptors[0] = client_fd; // changing the input to be the socket
}

// UNIX Domain Socket
void UDS_CLIENT_STREAM(char *path, int *descriptors)
{
    // open a UDS client to the server
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("error creating socket");
        sockets_terminator(descriptors);
        exit(1);
    }

    printf("UDS client\n");
    fflush(stdout);

    struct sockaddr_un server_addr;
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, path);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("error connecting to server");
        sockets_terminator(descriptors);
        exit(1);
    }

    descriptors[1] = sockfd; // changing the output to be the socket
}

// UNIX Domain Socket for datagram
void UDS_SERVER_DGRAM(char *path, int *descriptors)
{
    // opening a UDS server
    int sockfd = socket(AF_UNIX, SOCK_DGRAM, 0); // for datagram like udp protocol
    if (sockfd == -1)
    {
        perror("error creating socket");
        sockets_terminator(descriptors);
        exit(1);
    }

    printf("1.UDS socket created\n");
    struct sockaddr_un server_addr;
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, path);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("error binding socket");
        sockets_terminator(descriptors);
        exit(1);
    }

    printf("2.UDS binded\n");

    struct sockaddr_un client_addr;
    socklen_t client_len = sizeof(client_addr);

    char buffer[1024];
    int numbytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &client_len);
    if (numbytes == -1)
    {
        perror("error receiving data");
        sockets_terminator(descriptors);
        exit(1);
    }

    printf("3.UDS received data\n");

    // if (connect(sockfd, (struct sockaddr *)&client_addr, sizeof(client_addr)) == -1)
    // {
    //     perror("error connecting to client");
    //     sockets_terminator(descriptors);
    //     exit(1);
    // }

    // printf("3.UDS connected\n");

    // if (sendto(sockfd, "ACK", 3, 0, (struct sockaddr *)&client_addr, client_len) == -1)
    // {
    //     perror("error sending ACK");
    //     sockets_terminator(descriptors);
    //     exit(1);
    // }

    // printf("4.UDS sent ACK\n");

    descriptors[0] = sockfd; // changing the input to be the socket
}

// UNIX Domain Socket for datagram
void UDS_CLIENT_DGRAM(char *path, int *descriptors)
{
    // open a UDS client to the server
    int sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sockfd == -1)
    {
        perror("error creating socket");
        sockets_terminator(descriptors);
        exit(1);
    }

    printf("UDS client\n");
    fflush(stdout);

    struct sockaddr_un server_addr;
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, path);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("error connecting to server");
        sockets_terminator(descriptors);
        exit(1);
    }

    descriptors[1] = sockfd; // changing the output to be the socket
}

// for -i  nc localhost <port>
// for -o  nc -l -p <port>
// for -b  nc -l -p <port> | nc localhost <port>
// for -e  <program> <arguments>

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int opt;
    char *evalue = NULL;
    char *bvalue = NULL;
    char *ivalue = NULL;
    char *ovalue = NULL;
    char *tvalue = NULL;

    while ((opt = getopt(argc, argv, "e:b:i:o:t:")) != -1)
    {
        switch (opt)
        {
        case 'e':
            evalue = optarg;
            break;
        case 'b':
            bvalue = optarg;
            break;
        case 'i':
            ivalue = optarg;
            break;
        case 'o':
            ovalue = optarg;
            break;
        case 't':
            tvalue = optarg;
            break;
        default:
            fprintf(stderr, "Usage: %s <port>\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (tvalue != NULL)
    {
        signal(SIGALRM, handle_alarm);
        alarm(atoi(tvalue));
    }

    int descriptors[2];
    descriptors[0] = STDIN_FILENO;  // input fd
    descriptors[1] = STDOUT_FILENO; // output fd

    // -b cannot be used with -i or -o, not support multiple options
    if (bvalue != NULL && (ivalue != NULL || ovalue != NULL))
    {
        fprintf(stderr, "Option -b cannot be used with -i or -o\n");
        fprintf(stderr, "Usage: %s -e <value> [-b <value>] [-i <value>] [-o <value>]\n", argv[0]);
        exit(1);
    }

    // by default, the input and output are from/to the terminal
    // we will change it according to the socket input/output
    // int sock_input = STDIN_FILENO;
    // int sock_output = STDOUT_FILENO;

    if (ivalue != NULL)
    {
        // -i TCPS<port> or UDPS<port>
        // Now need to decied if to open TCP server or UDP server
        // first need to check the demand

        // strncpy(server_kind, ivalue, 4); // copying the first 4 characters to the server_kind
        // skip the "TCPS" prefix
        // taking the port, skipping the first 4 characters TCPSport
        printf("The i_value is: %s\n", ivalue);
        if (strncmp(ivalue, "TCPS", 4) == 0)
        {
            ivalue += 4;
            int port = atoi(ivalue);
            TCP_SERVER(descriptors, port, NULL);
        }
        else if (strncmp(ivalue, "UDPS", 4) == 0)
        {
            ivalue += 4;
            int port = atoi(ivalue);
            if (tvalue != NULL)
            {
                UDP_SERVER(descriptors, port, atoi(tvalue));
            }
            else
            {
                UDP_SERVER(descriptors, port, 0);
            }
        }
        else if (strncmp(ivalue, "UDSSS", 5) == 0)
        {
            ivalue += 5; // skip the prefix to give the correct path.
            printf("The path is: %s\n", ivalue);
            UDS_SERVER_STREAM(ivalue, descriptors);
        }

        else if (strncmp(ivalue, "UDSSD", 5) == 0)
        {
            ivalue += 5; // skip the prefix to give the correct path.
            UDS_SERVER_DGRAM(ivalue, descriptors);
        }

        else
        {
            fprintf(stderr, "i_value: Invalid input.\n");
            exit(1);
        }
    }

    if (ovalue != NULL) // changin the output to the one who we're addressing
    {

        if (strncmp(ovalue, "TCPC", 4) == 0)
        {
            ovalue += 4; // skip the "TCPS" prefix
            char *ip_server = strtok(ovalue, ",");
            // getting the ip like in the example TCPClocalhost,8080
            if (ip_server == NULL)
            {
                fprintf(stderr, "Invalid server IP\n");
                sockets_terminator(descriptors);
                exit(1);
            }
            // get the rest of the string after the comma, this is the port
            char *port_server = strtok(NULL, ",");
            if (port_server == NULL)
            {
                fprintf(stderr, "Invalid server port\n");
                sockets_terminator(descriptors);
                exit(1);
            }
            int port = atoi(port_server); // converting the port to integer
            TCP_client(descriptors, ip_server, port);
        }
        else if (strncmp(ovalue, "UDPC", 4) == 0)
        {
            ovalue += 4; // skip the "UDPC" prefix
            char *ip_server = strtok(ovalue, ",");
            if (ip_server == NULL)
            {
                fprintf(stderr, "Invalid server IP\n");
                sockets_terminator(descriptors);
                exit(1);
            }

            char *port_server = strtok(NULL, ",");
            if (port_server == NULL)
            {
                fprintf(stderr, "Invalid server port\n");
                sockets_terminator(descriptors);
                exit(1);
            }
            int port = atoi(port_server); // converting the port to integer
            UDP_CLIENT(descriptors, ip_server, port);
        }
        else if (strncmp(ovalue, "UDSCD", 5) == 0)
        {
            // unix domain sockets - client - datacram connect to path.
            ovalue += 5; // skip the "UDSCD" prefix
            UDS_CLIENT_DGRAM(ovalue, descriptors);
        }
        else if (strncmp(ovalue, "UDSCS", 5) == 0)
        {
            // unix domain sockets - client - stream connect to path.
            ovalue += 5; // skip the "UDSCS" prefix
            UDS_CLIENT_STREAM(ovalue, descriptors);
        }
        else
        {
            fprintf(stderr, "o_value: Invalid server kind.\n");
            sockets_terminator(descriptors);
            exit(1);
        }
    }

    if (bvalue != NULL) // changin the output to the one who we're addressing
    {
        if (strncmp(bvalue, "TCPS", 4) == 0)
        {
            bvalue += 4; // skip the "TCPS" prefix
            int port = atoi(bvalue);
            TCP_SERVER(descriptors, port, bvalue);
        }
        else if (strncmp(bvalue, "UDPS", 4) == 0)
        {
            bvalue += 4; // skip the "UDPS" prefix
            int port = atoi(bvalue);
            UDP_SERVER(descriptors, port, 0); // sets descriptors[0] to the socket
            descriptors[1] = descriptors[0];  // sets descriptors[1] to the socket
        }
        else if (strncmp(bvalue, "UDSSD", 5) == 0)
        {
            bvalue += 5; // skip the "UDSSD" prefix
            UDS_SERVER_DGRAM(bvalue, descriptors);
            descriptors[1] = descriptors[0]; // sets descriptors[1] to the socket
        }
        else if (strncmp(bvalue, "UDSSS", 5) == 0)
        {
            bvalue += 5; // skip the "UDSSS" prefix
            UDS_SERVER_STREAM(bvalue, descriptors);
            descriptors[1] = descriptors[0]; // sets descriptors[1] to the socket
        }
        else
        {
            fprintf(stderr, "b_value: Invalid server kind.\n");
            sockets_terminator(descriptors);
            exit(1);
        }
    }

    if (evalue != NULL)
    {
        // After finishinig changing the input and output, we're changing the input and output to the new socket
        if (descriptors[0] != STDIN_FILENO)
        {
            if (dup2(descriptors[0], STDIN_FILENO) == -1)
            {
                close(descriptors[0]);
                if (descriptors[1] != STDOUT_FILENO)
                {
                    close(descriptors[1]);
                }
                perror("dup2 input");
                exit(EXIT_FAILURE);
            }
        }
        if (descriptors[1] != STDOUT_FILENO)
        {
            if (dup2(descriptors[1], STDOUT_FILENO) == -1)
            {
                close(descriptors[1]);
                if (descriptors[0] != STDIN_FILENO)
                {
                    close(descriptors[0]);
                }
                perror("dup2 output");
                exit(EXIT_FAILURE);
            }
        }
        RUN(evalue); // this gets the whole command string and runs it
    }
    else //(evalue == NULL)
    {

        struct pollfd fds[4]; // poll file descriptors
        int nfds = 4;         // number of file descriptors

        fds[0].fd = descriptors[0]; // stdin
        fds[0].events = POLLIN;     // check for reading

        fds[1].fd = descriptors[1]; // input_fd
        fds[1].events = POLLIN;     // check for reading

        fds[2].fd = STDIN_FILENO; // stdin
        fds[2].events = POLLIN;   // check for reading

        fds[3].fd = STDOUT_FILENO; // stdout
        fds[3].events = POLLIN;    // check for reading

        while (1)
        {
            int ret = poll(fds, nfds, -1); // wait indefinitely for an event
            if (ret == -1)                 // poll failed
            {
                perror("poll");
                exit(EXIT_FAILURE);
            }

            // in case b is null we know that the values in descriptors[0] and descriptors[1] are not the same
            // and in this case we will always read from descriptors[0] and write to descriptors[1].
            if (bvalue == NULL && fds[0].revents & POLLIN)
            {
                char buffer[1024];
                int bytes_read = read(fds[0].fd, buffer, sizeof(buffer)); // read from the stdin
                if (bytes_read == -1)
                {
                    perror("read");
                    exit(EXIT_FAILURE);
                }
                if (bytes_read == 0)
                {
                    break;
                }
                if (write(fds[1].fd, buffer, bytes_read) == -1)
                {
                    perror("write");
                    exit(EXIT_FAILURE);
                }
            }

            // in case b is not null we know that the values in descriptors[0] and descriptors[1] are the same
            // so we need to seperate in which case we are reading from stdin or from the input_fd
            // and write to stdout or to the output_fd
            if (bvalue != NULL && fds[1].revents & POLLIN) // input_fd has data to read
            {
                char buffer[1024];
                int bytes_read = read(fds[1].fd, buffer, sizeof(buffer)); // read from the input_fd
                if (bytes_read == -1)
                {
                    perror("read");
                    exit(EXIT_FAILURE);
                }
                if (bytes_read == 0)
                {
                    break;
                }
                if (write(fds[3].fd, buffer, bytes_read) == -1)
                {
                    perror("write");
                    exit(EXIT_FAILURE);
                }
            }
            if (bvalue != NULL && fds[2].revents & POLLIN) // stdin has data to read
            {
                char buffer[1024];
                int bytes_read = read(fds[2].fd, buffer, sizeof(buffer)); // read from stdin
                if (bytes_read == -1)
                {
                    perror("read");
                    exit(EXIT_FAILURE);
                }
                if (bytes_read == 0)
                {
                    break;
                }
                if (write(descriptors[1], buffer, bytes_read) == -1)
                {
                    perror("write");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }

    close(descriptors[0]);
    close(descriptors[1]);

    return 0;
}
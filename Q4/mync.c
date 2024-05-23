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
        exit(1);
    }

    // to start showing the game before we're sending to ourself an ack to show the first move
    if (sendto(sockfd, "ACK", 3, 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("error sending ACK");
        exit(1);
    }

    // read the data from the client
    char buffer[2];
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int numbytes = recvfrom(sockfd, buffer, 2, 0, (struct sockaddr *)&client_addr, &client_addr_len);
    if (numbytes == -1)
    {
        perror("error receiving data");
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

    // send data to the server
    if (sendto(sockfd, "ACK", 3, 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("error sending data");
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

    signal(SIGALRM, handle_alarm);

    if (evalue == NULL)
    {
        fprintf(stderr, "Usage: %s -e \"<program> <arguments>\"\n", argv[0]);
        exit(EXIT_FAILURE);
    }

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

    int descriptors[2];
    descriptors[0] = STDIN_FILENO;
    descriptors[1] = STDOUT_FILENO;

    if (ivalue != NULL)
    {
        // -i TCPS<port> or UDPS<port>
        // Now need to decied if to open TCP server or UDP server
        // first need to check the demand
        char server_kind[4] = {0};
        strncpy(server_kind, ivalue, 4); // copying the first 4 characters to the server_kind
        int port = atoi(ivalue += 4);    // taking the port, skipping the first 4 characters TCPSport
        if (strcmp(server_kind, "TCPS") == 0)
        {
            TCP_SERVER(descriptors, port, NULL);
            // open fork
            // read the data and sent it to th ttt
            // char buffer[2];
            // while (read(newsockfd, buffer, 1) > 0)
            // {
            //     printf("Received number: %c\n", buffer[0]);
            //     write(pipefd[1], buffer, 1);
            // }
        }
        else if (strcmp(server_kind, "UDPS") == 0)
        {
            if (tvalue != NULL)
            {
                UDP_SERVER(descriptors, port, atoi(tvalue));
            }
            else
            {
                UDP_SERVER(descriptors, port, 0);
            }
        }
    }

    if (ovalue != NULL) // changin the output to the one who we're addressing
    {

        char server_kind[4] = {0};
        strncpy(server_kind, ovalue, 4);       // copying the first 4 characters to the server_kind
        ovalue += 4;                           // skip the "TCPC" prefix
        char *ip_server = strtok(ovalue, ","); // getting the ip like in the example TCPClocalhost,8080
        if (ip_server == NULL)
        {
            fprintf(stderr, "Invalid server IP\n");
            exit(1);
        }
        // get the rest of the string after the comma, this is the port
        char *port_server = strtok(NULL, ",");
        if (port_server == NULL)
        {
            fprintf(stderr, "Invalid server port\n");
            exit(1);
        }

        int port = atoi(port_server); // converting the port to integer

        if (strcmp(server_kind, "TCPC") == 0)
        {
            TCP_client(descriptors, ip_server, port);
        }

        else if (strcmp(server_kind, "UDPC") == 0) // creating UDP client
        {
            // ????????????????????????????
        }
    }

    if (bvalue != NULL) // changin the output to the one who we're addressing
    {
        char server_kind[4] = {0};
        // open TCP server to listen to the port
        strncpy(server_kind, bvalue, 4); // copying the first 4 characters to the server_kind
        bvalue += 4;                     // skip the "TCPS" prefix
        int port = atoi(bvalue);
        if (strcmp(server_kind, "TCPS") == 0)
        {
            TCP_SERVER(descriptors, port, bvalue);
        }
    }
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

    RUN(evalue); // this getting the whole char of command to run
    close(descriptors[0]);
    close(descriptors[1]);

    return 0;
}
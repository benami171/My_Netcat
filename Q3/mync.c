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

    int pipefd[2];

    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    while ((opt = getopt(argc, argv, "e:b:i:o")) != -1)
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
        default:
            fprintf(stderr, "Usage: %s <port>\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

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
    int sock_input = STDIN_FILENO;
    int sock_output = STDOUT_FILENO;

    if (ivalue != NULL)
    {
        // listeing to input form client on local host, and printing to stdout

        int port = atoi(ivalue += 4); // taking the port, skipping the first 4 characters TCPS
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
        {
            perror("socket");
            exit(EXIT_FAILURE);
        }

        // allow the socket to be reused, maybe to change to fork?
        int optval = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
        {
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // listen to any address

        // if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0)
        // {
        //     perror("inet_pton");
        //     exit(EXIT_FAILURE);
        // }

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

        // maybe to open ehre fork?
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        // chanign the input_fd to the new socket after accepting the connection
        sock_input = accept(sockfd, (struct sockaddr *)&client_addr, &client_len);
        if (sock_input < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        // open fork
        // read the data and sent it to th ttt
        // char buffer[2];
        // while (read(newsockfd, buffer, 1) > 0)
        // {
        //     printf("Received number: %c\n", buffer[0]);
        //     write(pipefd[1], buffer, 1);
        // }
    }

    if (ovalue != NULL) // changin the output to the one who we're addressing
    {
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

        // open a TCP client to the server
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1)
        {
            perror("error creating socket");
            return 1;
        }

        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);

        if (inet_pton(AF_INET, ip_server, &server_addr) <= 0)
        {
            perror("Invalid address/ Address not supported");
            return 1;
        }

        // connecting to the server
        if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
        {
            perror("error connecting to server");
            if (sock_input != STDIN_FILENO)
            {
                close(sock_input);
            }
            return 1;
        }

        sock_output = sock; // changin the output to form the socket to the client
    }

    if (bvalue != NULL)
    {
        // open TCP server to listen to the port
        bvalue += 4; // skip the "TCPS" prefix
        int port = atoi(bvalue);

        // create TCP socket that will listen to input on localhost and port
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1)
        {
            perror("error creating socket");
            exit(EXIT_FAILURE);
        }

        // allow the socket to be reused
        int optval = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
        {
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }

        // bind the socket to the address
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);

        if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
        {
            perror("error binding socket");
            exit(EXIT_FAILURE);
        }

        // listen for incoming connections - at most 1
        if (listen(sockfd, 1) == -1)
        {
            perror("error listening on socket");
            exit(EXIT_FAILURE);
        }

        // accept the connection and change the input_fd to the new socket
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int new_descriptoer = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (new_descriptoer == -1)
        {
            perror("error accepting connection");
            exit(EXIT_FAILURE);
        }

        // we want to send the input and output to the client
        sock_input = new_descriptoer;
        sock_output = new_descriptoer;
    }

    if (sock_input != STDIN_FILENO)
    {
        if (dup2(sock_input, STDIN_FILENO) == -1)
        {
            close(sock_input);
            if (sock_output != STDOUT_FILENO)
            {
                close(sock_output);
            }
            perror("dup2 input");
            exit(EXIT_FAILURE);
        }
    }
    if (sock_output != STDOUT_FILENO)
    {
        if (dup2(sock_output, STDOUT_FILENO) == -1)
        {
            close(sock_output);
            if (sock_input != STDIN_FILENO)
            {
                close(sock_input);
            }
            perror("dup2 output");
            exit(EXIT_FAILURE);
        }
    }

    pid_t pid = fork();
    if (pid == 0)
    {
        // Child process
        close(pipefd[1]);               // Close unused write end
        dup2(pipefd[0], STDIN_FILENO);  // Redirect standard input to read end of pipe
        execlp("./ttt", "./ttt", NULL); // Execute ttt game
        perror("execlp");
        exit(EXIT_FAILURE);
    }
    else
    {
        // Parent process
        close(pipefd[0]); // Close unused read end
        char buffer[2];
        while (fgets(buffer, sizeof(buffer), stdin))
        {
            write(pipefd[1], buffer, 1); // Write client's input to write end of pipe
        }
        close(pipefd[1]); // Close write end of pipe when done
        wait(NULL);       // Wait for child process to finish
    }
    return 0;
}
/**
 * #include <arpa/inet.h>
#include <getopt.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

void run_program(char *args_as_string) {
    // tokenize the string - split by space
    char *token = strtok(args_as_string, " ");

    if (token == NULL) {
        fprintf(stderr, "No arguments provided\n");
        exit(1);
    }
    // create an array of strings to store the arguments
    char *args = (char *)malloc(sizeof(char *));
    int n = 0;          // number of arguments
    args[n++] = token;  // add the first argument

    // get the rest of the arguments
    while (token != NULL) {
        token = strtok(NULL, " ");                                // get the next token (NULL - take the next token from the previous string)
        args = (char **)realloc(args, (n + 1) * sizeof(char *));  // allocate memory for the new argument
        args[n++] = token;                                        // add the new argument and increment the number of arguments
    }

    // fork and execute the program
    int fd = fork();
    if (fd < 0) {  // fork failed
        fprintf(stderr, "Fork failed\n");
        exit(1);
    }

    if (fd == 0) {  // child process
        execvp(args[0], args);
        fprintf(stderr, "Exec failed\n");
        free(args);
        exit(1);
    } else {
        wait(NULL);  // wait for the child process to finish
        // free the memory
        free(args);
        fflush(stdout);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s -e <value> [-b <value>] [-i <value>] [-o <value>]\n", argv[0]);
        exit(1);
    }

    int opt;
    char *e_value = NULL;
    char *b_value = NULL;
    char *i_value = NULL;
    char *o_value = NULL;

    while ((opt = getopt(argc, argv, "e:b:i:o:")) != -1) {
        switch (opt) {
            case 'e':
                e_value = optarg;
                break;
            case 'b':
                b_value = optarg;
                break;
            case 'i':
                i_value = optarg;
                break;
            case 'o':
                o_value = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s -e <value> [-b <value>] [-i <value>] [-o <value>]\n", argv[0]);
                exit(1);
        }
    }

    if (e_value == NULL) {
        fprintf(stderr, "Option -e is required\n");
        fprintf(stderr, "Usage: %s -e <value> [-b <value>] [-i <value>] [-o <value>]\n", argv[0]);
        exit(1);
    }

    if (b_value != NULL && (i_value != NULL || o_value != NULL)) {
        fprintf(stderr, "Option -b cannot be used with -i or -o\n");
        fprintf(stderr, "Usage: %s -e <value> [-b <value>] [-i <value>] [-o <value>]\n", argv[0]);
        exit(1);
    }

    int input_fd = STDIN_FILENO;
    int output_fd = STDOUT_FILENO;

    if (i_value != NULL) {
        i_value += 4;  // skip the "TCPS" prefix
        int port = atoi(i_value);

        // create TCP socket that will listen to input on localhost:port
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            perror("error creating socket");
            exit(EXIT_FAILURE);
        }

        // allow the socket to be reused
        int optval = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }

        // bind the socket to the address

        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);

        if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
            perror("error binding socket");
            exit(EXIT_FAILURE);
        }

        // listen for incoming connections - at most 1
        if (listen(sockfd, 1) == -1) {
            perror("error listening on socket");
            exit(EXIT_FAILURE);
        }

        // accept the connection and change the input_fd to the new socket
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        input_fd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (input_fd == -1) {
            perror("error accepting connection");
            exit(EXIT_FAILURE);
        }
    }

    if (o_value != NULL) {
        o_value += 4;  // skip the "TCPC" prefix
        char *server_ip = strtok(o_value, ",");
        if (server_ip == NULL) {
            fprintf(stderr, "Invalid server IP\n");
            exit(1);
        }
        // get the rest of the string after the comma
        char *server_port = strtok(NULL, ",");
        if (server_port == NULL) {
            fprintf(stderr, "Invalid server port\n");
            exit(1);
        }
        int port = atoi(server_port);

        // open a TCP client to the server
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            perror("error creating socket");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);

        if (inet_pton(AF_INET, server_ip, &addr.sin_addr) <= 0) {
            perror("Invalid address/ Address not supported");
            exit(EXIT_FAILURE);
        }

        if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
            perror("error connecting to server");
            if (input_fd != STDIN_FILENO) {
                close(input_fd);
            }
            exit(EXIT_FAILURE);
        }

        output_fd = sockfd;
    }

    if (b_value != NULL) {
        // open TCP server to listen to the port
        b_value += 4;  // skip the "TCPS" prefix
        int port = atoi(b_value);

        // create TCP socket that will listen to input on localhost:port
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            perror("error creating socket");
            exit(EXIT_FAILURE);
        }

        // allow the socket to be reused
        int optval = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }

        // bind the socket to the address
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);

        if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
            perror("error binding socket");
            exit(EXIT_FAILURE);
        }

        // listen for incoming connections - at most 1
        if (listen(sockfd, 1) == -1) {
            perror("error listening on socket");
            exit(EXIT_FAILURE);
        }

        // accept the connection and change the input_fd to the new socket
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int new_fd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (new_fd == -1) {
            perror("error accepting connection");
            exit(EXIT_FAILURE);
        }

        input_fd = new_fd;
        output_fd = new_fd;
    }

    if (input_fd != STDIN_FILENO) {
        if (dup2(input_fd, STDIN_FILENO) == -1) {
            close(input_fd);
            if (output_fd != STDOUT_FILENO) {
                close(output_fd);
            }
            perror("dup2 input");
            exit(EXIT_FAILURE);
        }
    }
    if (output_fd != STDOUT_FILENO) {
        if (dup2(output_fd, STDOUT_FILENO) == -1) {
            close(output_fd);
            if (input_fd != STDIN_FILENO) {
                close(input_fd);
            }
            perror("dup2 output");
            exit(EXIT_FAILURE);
        }
    }
    run_program(e_value);
    //TODO: check how to close the sockets
    return 0;
}
*/

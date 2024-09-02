#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>


// gets a string like: "./ttt 123654789"
void RUN(char *args_as_string) {

    // tokenize the string - split by space
    char *token = strtok(args_as_string, " "); // ./ttt
    char *arguments = strtok(NULL, ""); // 123456789

    if (token == NULL || arguments == NULL) {
        fprintf(stderr, "No arguments provided\n");
        exit(1);
    }

    char *args[] = {token, arguments, NULL};

    // fork and execute the program
    int fd = fork();
    if (fd < 0) {  // fork failed
        fprintf(stderr, "Fork failed\n");
        exit(1);
    }

    if (fd == 0) {  // child process
        execvp(args[0], args);
        fprintf(stderr, "Exec failed\n");
        exit(1);
    } else {
        wait(NULL);  // wait for the child process to finish
        fflush(stdout);
    }
}

// called only when there was a problem throughout the program
void sockets_terminator(int *descriptors) {
    if (descriptors[0] != STDIN_FILENO) {
        close(descriptors[0]);
    }
    if (descriptors[1] != STDOUT_FILENO) {
        close(descriptors[1]);
    }
}

void handle_alarm(int sig) {
    // Terminate the process
    exit(1);
}

// sending the descriptor to handel, and the portnumber to open the server on
void TCP_SERVER(int *descriptors, int port, char *b_flag, int flag) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        sockets_terminator(descriptors);
        exit(EXIT_FAILURE);
    }
    printf("TCP socket created\n");

    // allow the socket to be reused, maybe to change to fork?
    int optval = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
        perror("setsockopt");
        close(sockfd);
        sockets_terminator(descriptors);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // listen to any address

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(sockfd);
        sockets_terminator(descriptors);
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, 1) < 0) {
        perror("listen");
        sockets_terminator(descriptors);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    // chanigng the input_fd to the new socket after accepting the connection
    int client_fd = accept(sockfd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd < 0) {
        perror("accept");
        sockets_terminator(descriptors);
        exit(EXIT_FAILURE);
    }

    if (flag == 0) {
        descriptors[0] = client_fd;
    } else {
        descriptors[1] = client_fd;
    }
    if (b_flag != NULL) {
        descriptors[1] = client_fd;
    }
    close(sockfd);
}

void TCP_client(int *descriptors, char *ip, int port, char *bvalue, int flag) {
    // open a TCP client to the server
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("error creating socket");
        sockets_terminator(descriptors);
        exit(1);
    }

    printf("TCP client\n");
    fflush(stdout);

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) {
        perror("setsockopt");
        sockets_terminator(descriptors);
        exit(1);
    }

    if (strncmp(ip, "localhost", 9) == 0) {
        ip = "127.0.0.1";
    }

    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        sockets_terminator(descriptors);
        exit(1);
    }

    // connecting to the server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("error connecting to server");
        if (descriptors[0] != STDIN_FILENO)  // to ensure we'rent getting input from another place
        {
            close(descriptors[0]);
        }
        sockets_terminator(descriptors);
        exit(1);
    }

    if (flag == 0) {
        descriptors[1] = sock;  // changing the output to form the socket to the client
    } else {
        descriptors[0] = sock;  // changing the input to form the socket to the client
    }

    if (bvalue != NULL) {
        descriptors[0] = sock;  // changing the input to form the socket to the client
    }
}

void UDP_SERVER(int *descriptors, int port, int timeout, int flag) {
    // open a UDP server to listen to the port
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("error creating socket");
        sockets_terminator(descriptors);
        exit(1);
    }
    printf("UDP Socket created\n");

    // if not set, the port will be in use for 2 minutes after the program ends
    int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("setsockopt failed");
        sockets_terminator(descriptors);
        exit(1);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("error binding socket");
        sockets_terminator(descriptors);
        exit(1);
    }

    // read the data from the client
    char buffer[1024];
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    int numbytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &client_addr_len);
    if (numbytes == -1) {
        perror("error receiving data");
        sockets_terminator(descriptors);
        exit(1);
    }

    if (connect(sockfd, (struct sockaddr *)&client_addr, sizeof(client_addr)) == -1) {
        perror("error connecting to client");
        sockets_terminator(descriptors);
        exit(1);
    }

    if (sendto(sockfd, "ACK", 3, 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("error sending ACK");
        exit(1);
    }

    if (flag == 0) {
        descriptors[0] = sockfd;  // changing the descriptor to be the socket
    } else {
        descriptors[1] = sockfd;  // changing the descriptor to be the socket
    }
    alarm(timeout);
}

void UDP_CLIENT(int *descriptors, char *ip, int port, int flag) {
    // open a UDP client to the server
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("error creating socket");
        exit(1);
    }
    printf("UDP client\n");
    fflush(stdout);

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(1);
    }

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("error creating socket");
        exit(EXIT_FAILURE);
    }
    printf(" the server ip is %s\n", ip);
    printf(" the server port is %d\n", port);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("error connecting to server");
        exit(1);
    }

    // Send a message to the server
    char *message = "Lets play !\n";
    if (sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("error sending message");
        exit(1);
    }

    if (flag == 0) {
        descriptors[1] = sockfd;  // changing the output to be the socket
    } else {
        descriptors[0] = sockfd;  // changing the input to be the socket
    }
}

// UNIX Domain Socket
void UDS_SERVER_STREAM(char *path, int *descriptors) {
    // opening a UDS server
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);  // for stream like tcp protocol
    if (sockfd == -1) {
        sockets_terminator(descriptors);
        perror("error creating socket");
        exit(1);
    }

    printf("1.UDS socket created\n");
    struct sockaddr_un server_addr;      // creating the server address
    server_addr.sun_family = AF_UNIX;    // setting the family to be UNIX
    strcpy(server_addr.sun_path, path);  // setting the path to be the path we got from the input

    // binding the socket to the server address
    unlink(path);  // remove the file if it exists
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("error binding socket");
        sockets_terminator(descriptors);
        exit(1);
    }

    printf("2.UDS binded\n");

    // listening to incoming connections, we set it to be 1 connection at most.
    if (listen(sockfd, 1) == -1) {
        perror("error listening");
        sockets_terminator(descriptors);
        exit(1);
    }

    printf("3.UDS listening\n");

    // creating client struct to store the client address and accepting the connection
    struct sockaddr_un client_addr;
    socklen_t client_len = sizeof(client_addr);

    int client_fd = accept(sockfd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd == -1) {
        perror("error accepting connection");
        sockets_terminator(descriptors);
        exit(1);
    }

    printf("4.UDS accepted\n");

    descriptors[0] = client_fd;  // changing the input to be the socket
}

// UNIX Domain Socket
void UDS_CLIENT_STREAM(char *path, int *descriptors) {
    // open a UDS client to the server
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("error creating socket");
        sockets_terminator(descriptors);
        exit(1);
    }

    printf("UDS client\n");
    fflush(stdout);

    struct sockaddr_un server_addr;
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, path);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("error connecting to server");
        sockets_terminator(descriptors);
        exit(1);
    }

    descriptors[1] = sockfd;  // changing the output to be the socket
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int opt;
    char *evalue = NULL;
    char *bvalue = NULL;
    char *ivalue = NULL;
    char *ovalue = NULL;
    char *tvalue = NULL;

    while ((opt = getopt(argc, argv, "e:b:i:o:t:")) != -1) {
        switch (opt) {
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

    if (tvalue != NULL) {
        signal(SIGALRM, handle_alarm);  // setting the signal handler to get alarm as its signal
                                        // and handle it with the handle_alarm function
        alarm(atoi(tvalue));            // setting a timer that when expired, sends a SIGALRM signal to the process,
                                        // triggering the handle_alarm function.
    }

    int descriptors[2];
    descriptors[0] = STDIN_FILENO;   // input fd
    descriptors[1] = STDOUT_FILENO;  // output fd

    // -b cannot be used with -i or -o, not support multiple options
    if (bvalue != NULL && (ivalue != NULL || ovalue != NULL)) {
        fprintf(stderr, "Option -b cannot be used with -i or -o\n");
        fprintf(stderr, "Usage: %s -e <value> [-b <value>] [-i <value>] [-o <value>]\n", argv[0]);
        exit(1);
    }

    if (ivalue != NULL) {
        printf("The i_value is: %s\n", ivalue);
        if (strncmp(ivalue, "TCPS", 4) == 0) {
            ivalue += 4;
            int port = atoi(ivalue);
            TCP_SERVER(descriptors, port, NULL, 0);
        } else if (strncmp(ivalue, "UDPS", 4) == 0) {
            ivalue += 4;
            int port = atoi(ivalue);
            if (tvalue != NULL) {
                UDP_SERVER(descriptors, port, atoi(tvalue), 0);
            } else {
                UDP_SERVER(descriptors, port, 0, 0);
            }
        } else if (strncmp(ivalue, "UDSSS", 5) == 0) {
            ivalue += 5;  // skip the prefix to give the correct path.
            printf("The path is: %s\n", ivalue);
            UDS_SERVER_STREAM(ivalue, descriptors);
        }

        else if (strncmp(ivalue, "UDSCS", 5) == 0) {
            ivalue += 5;  // skip the prefix to give the correct path.
            UDS_CLIENT_STREAM(ivalue, descriptors);
            descriptors[0] = descriptors[1];
            descriptors[1] = STDOUT_FILENO;
        }

        else if (strncmp(ivalue, "TCPC", 4) == 0) {
            ivalue += 4;  // skip the "TCPS" prefix
            char *ip_server = strtok(ivalue, ",");
            // getting the ip like in the example TCPClocalhost,8080
            if (ip_server == NULL) {
                fprintf(stderr, "Invalid server IP\n");
                sockets_terminator(descriptors);
                exit(1);
            }
            // get the rest of the string after the comma, this is the port
            char *port_server = strtok(NULL, ",");
            if (port_server == NULL) {
                fprintf(stderr, "Invalid server port\n");
                sockets_terminator(descriptors);
                exit(1);
            }
            int port = atoi(port_server);  // converting the port to integer
            TCP_client(descriptors, ip_server, port, NULL, 1);
        }

        else if (strncmp(ivalue, "UDPC", 4) == 0) {
            ivalue += 4;  // skip the "UDPC" prefix
            char *ip_server = strtok(ivalue, ",");
            if (ip_server == NULL) {
                fprintf(stderr, "Invalid server IP\n");
                sockets_terminator(descriptors);
                exit(1);
            }

            char *port_server = strtok(NULL, ",");
            if (port_server == NULL) {
                fprintf(stderr, "Invalid server port\n");
                sockets_terminator(descriptors);
                exit(1);
            }
            int port = atoi(port_server);  // converting the port to integer
            UDP_CLIENT(descriptors, ip_server, port, 1);
        }

        else {
            fprintf(stderr, "i_value: Invalid input.\n");
            exit(1);
        }
    }

    if (ovalue != NULL) {  // changin the output to the one who we're addressing

        if (strncmp(ovalue, "TCPC", 4) == 0) {
            ovalue += 4;  // skip the "TCPS" prefix
            char *ip_server = strtok(ovalue, ",");
            // getting the ip like in the example TCPClocalhost,8080
            if (ip_server == NULL) {
                fprintf(stderr, "Invalid server IP\n");
                sockets_terminator(descriptors);
                exit(1);
            }
            // get the rest of the string after the comma, this is the port
            char *port_server = strtok(NULL, ",");
            if (port_server == NULL) {
                fprintf(stderr, "Invalid server port\n");
                sockets_terminator(descriptors);
                exit(1);
            }
            int port = atoi(port_server);  // converting the port to integer
            TCP_client(descriptors, ip_server, port, NULL, 0);
        } else if (strncmp(ovalue, "UDPC", 4) == 0) {
            ovalue += 4;  // skip the "UDPC" prefix
            char *ip_server = strtok(ovalue, ",");
            if (ip_server == NULL) {
                fprintf(stderr, "Invalid server IP\n");
                sockets_terminator(descriptors);
                exit(1);
            }

            char *port_server = strtok(NULL, ",");
            if (port_server == NULL) {
                fprintf(stderr, "Invalid server port\n");
                sockets_terminator(descriptors);
                exit(1);
            }
            int port = atoi(port_server);  // converting the port to integer
            UDP_CLIENT(descriptors, ip_server, port, 0);
        }

        else if (strncmp(ovalue, "UDSCS", 5) == 0) {
            // unix domain sockets - client - stream connect to path.
            ovalue += 5;  // skip the "UDSCS" prefix
            UDS_CLIENT_STREAM(ovalue, descriptors);
        }

        else if (strncmp(ovalue, "TCPS", 4) == 0) {
            ovalue += 4;  // skip the "TCPS" prefix
            int port = atoi(ovalue);
            TCP_SERVER(descriptors, port, NULL, 1);
        } else if (strncmp(ovalue, "UDPS", 4) == 0) {
            ovalue += 4;  // skip the "UDPS" prefix
            int port = atoi(ovalue);
            if (tvalue != NULL) {
                UDP_SERVER(descriptors, port, atoi(tvalue), 1);
            } else {
                UDP_SERVER(descriptors, port, 0, 1);
            }
        } else if (strncmp(ovalue, "UDSSS", 5) == 0) {
            ovalue += 5;  // skip the "UDSSS" prefix
            UDS_SERVER_STREAM(ovalue, descriptors);
            descriptors[1] = descriptors[0];  // sets descriptors[1] to the socket
            descriptors[0] = STDIN_FILENO;
        }

        else {
            fprintf(stderr, "o_value: Invalid server kind.\n");
            sockets_terminator(descriptors);
            exit(1);
        }
    }

    if (bvalue != NULL)  // changin the output to the one who we're addressing
    {
        if (strncmp(bvalue, "TCPS", 4) == 0) {
            bvalue += 4;  // skip the "TCPS" prefix
            int port = atoi(bvalue);
            TCP_SERVER(descriptors, port, bvalue, 0);
        } else if (strncmp(bvalue, "TCPC", 4) == 0) {
            bvalue += 4;  // skip the "TCPC" prefix
            char *ip_server = strtok(bvalue, ",");
            // getting the ip like in the example TCPClocalhost,8080
            if (ip_server == NULL) {
                fprintf(stderr, "Invalid server IP\n");
                sockets_terminator(descriptors);
                exit(1);
            }
            // get the rest of the string after the comma, this is the port
            char *port_server = strtok(NULL, ",");
            if (port_server == NULL) {
                fprintf(stderr, "Invalid server port\n");
                sockets_terminator(descriptors);
                exit(1);
            }
            int port = atoi(port_server);  // converting the port to integer
            TCP_client(descriptors, ip_server, port, bvalue, 0);
        } else if (strncmp(bvalue, "UDPC", 4) == 0) {
            bvalue += 4;  // skip the "UDPC" prefix
            char *ip_server = strtok(bvalue, ",");
            if (ip_server == NULL) {
                fprintf(stderr, "Invalid server IP\n");
                sockets_terminator(descriptors);
                exit(1);
            }

            char *port_server = strtok(NULL, ",");
            if (port_server == NULL) {
                fprintf(stderr, "Invalid server port\n");
                sockets_terminator(descriptors);
                exit(1);
            }
            int port = atoi(port_server);  // converting the port to integer
            UDP_CLIENT(descriptors, ip_server, port, 0);
            descriptors[0] = descriptors[1];  // sets descriptors[0] to the socket
        } else if (strncmp(bvalue, "UDPS", 4) == 0) {
            bvalue += 4;  // skip the "UDPS" prefix
            int port = atoi(bvalue);
            UDP_SERVER(descriptors, port, 0, 0);  // sets descriptors[0] to the socket
            descriptors[1] = descriptors[0];      // sets descriptors[1] to the socket
        }

        else if (strncmp(bvalue, "UDSSS", 5) == 0)  // SERVER STREAM
        {
            bvalue += 5;  // skip the "UDSSS" prefix
            UDS_SERVER_STREAM(bvalue, descriptors);
            descriptors[1] = descriptors[0];  // sets descriptors[1] to the socket
        }

        else if (strncmp(bvalue, "UDSCS", 5) == 0) {
            bvalue += 5;  // skip the "UDSCS" prefix
            UDS_CLIENT_STREAM(bvalue, descriptors);
            descriptors[0] = descriptors[1];  // sets descriptors[0] to the socket
        }

        else {
            fprintf(stderr, "b_value: Invalid server kind.\n");
            sockets_terminator(descriptors);
            exit(1);
        }
    }

    if (evalue != NULL) {
        // After finishinig changing the input and output, we're changing the input and output to the new socket
        if (descriptors[0] != STDIN_FILENO) {
            if (dup2(descriptors[0], STDIN_FILENO) == -1) {
                close(descriptors[0]);
                if (descriptors[1] != STDOUT_FILENO) {
                    close(descriptors[1]);
                }
                perror("dup2 input");
                sockets_terminator(descriptors);
                exit(EXIT_FAILURE);
            }
        }
        if (descriptors[1] != STDOUT_FILENO) {
            if (dup2(descriptors[1], STDOUT_FILENO) == -1) {
                close(descriptors[1]);
                if (descriptors[0] != STDIN_FILENO) {
                    close(descriptors[0]);
                }
                perror("dup2 output");
                sockets_terminator(descriptors);
                exit(EXIT_FAILURE);
            }
        }
        printf("The e_value is: %s\n", evalue);
        RUN(evalue);  // this gets the whole command string and runs it
    } else            // (evalue == NULL)
    {
        printf("No -e command provided\n");
        struct pollfd fds[4];  // poll file descriptors
        int nfds = 4;          // number of file descriptors

        fds[0].fd = descriptors[0];  // input_fd
        fds[0].events = POLLIN;      // check for reading

        fds[1].fd = descriptors[1];  // output_fd
        fds[1].events = POLLIN;      // check for reading

        fds[2].fd = STDIN_FILENO;  // stdin
        fds[2].events = POLLIN;    // check for reading

        fds[3].fd = STDOUT_FILENO;  // stdout
        fds[3].events = POLLIN;     // check for reading

        while (1) {
            int ret = poll(fds, nfds, -1);  // wait indefinitely for an event
            if (ret == -1)                  // poll failed
            {
                perror("poll");
                sockets_terminator(descriptors);
                exit(EXIT_FAILURE);
            }
            
            // in case b is null we know that the values in descriptors[0] and descriptors[1] are not the same
            // and in this case we will always read from descriptors[0] and write to descriptors[1].
            if (bvalue == NULL && fds[0].revents & POLLIN) {
                char buffer[1024];
                int bytes_read = read(fds[0].fd, buffer, sizeof(buffer));  // read from the input_fd
                if (bytes_read == -1) {
                    perror("read");
                    sockets_terminator(descriptors);
                    exit(EXIT_FAILURE);
                }
                if (bytes_read == 0) {
                    break;
                }
                if (write(fds[1].fd, buffer, bytes_read) == -1) { // write to the output_fd
                    perror("write");
                    sockets_terminator(descriptors);
                    exit(EXIT_FAILURE);
                }
            }

            // in case b is not null we know that the values in descriptors[0] and descriptors[1] are the same
            // so we need to seperate in which case we are reading from stdin or from the input_fd
            // and write to stdout or to the output_fd
            if (bvalue != NULL && fds[1].revents & POLLIN)  // output_fd has data to read
            {
                char buffer[1024];
                int bytes_read = read(fds[1].fd, buffer, sizeof(buffer));  // read from the output_fd
                if (bytes_read == -1) {
                    perror("read");
                    sockets_terminator(descriptors);
                    exit(EXIT_FAILURE);
                }
                if (bytes_read == 0) {
                    break;
                }
                if (write(fds[3].fd, buffer, bytes_read) == -1) {
                    perror("write");
                    sockets_terminator(descriptors);
                    exit(EXIT_FAILURE);
                }
            }
            if (bvalue != NULL && fds[2].revents & POLLIN)  // stdin has data to read
            {
                char buffer[1024];
                int bytes_read = read(fds[2].fd, buffer, sizeof(buffer));  // read from stdin
                if (bytes_read == -1) {
                    perror("read");
                    sockets_terminator(descriptors);
                    exit(EXIT_FAILURE);
                }
                if (bytes_read == 0) {
                    break;
                }
                if (write(descriptors[1], buffer, bytes_read) == -1) {
                    perror("write");
                    sockets_terminator(descriptors);
                    exit(EXIT_FAILURE);
                }
            }
        }
    }

    close(descriptors[0]);
    close(descriptors[1]);

    return 0;
}
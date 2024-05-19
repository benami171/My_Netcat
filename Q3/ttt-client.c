#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    // int input_fd = STDIN_FILENO;
    // int output_fd = STDOUT_FILENO;
    // int server_fd = -1;
    // int client_fd = -1;
    // int port;

    // // Parse command line arguments
    // for (int i = 1; i < argc; i++) {
    //     if (strcmp(argv[i], "-i") == 0) {
    //         // Input from socket
    //         i++;
    //         if (i < argc) {
    //             int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    //             if (sockfd < 0) {
    //                 perror("socket");
    //                 exit(EXIT_FAILURE);
    //             }

    //             struct sockaddr_in server_addr;
    //             memset(&server_addr, 0, sizeof(server_addr));
    //             server_addr.sin_family = AF_INET;
    //             server_addr.sin_port = htons(atoi(argv[i]));
    //             server_addr.sin_addr.s_addr = INADDR_ANY;

    //             if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    //                 perror("bind");
    //                 exit(EXIT_FAILURE);
    //             }

    //             if (listen(sockfd, 1) < 0) {
    //                 perror("listen");
    //                 exit(EXIT_FAILURE);
    //             }

    //             client_fd = accept(sockfd, NULL, NULL);
    //             if (client_fd < 0) {
    //                 perror("accept");
    //                 exit(EXIT_FAILURE);
    //             }

    //             close(sockfd);
    //             input_fd = client_fd;
    //         }
    //     } else if (strcmp(argv[i], "-o") == 0) {
    //         // Output to socket
    //         i++;
    //         if (i < argc) {
    //             int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    //             if (sockfd < 0) {
    //                 perror("socket");
    //                 exit(EXIT_FAILURE);
    //             }

    //             struct sockaddr_in server_addr;
    //             memset(&server_addr, 0, sizeof(server_addr));
    //             server_addr.sin_family = AF_INET;
    //             server_addr.sin_port = htons(atoi(argv[i]));
    //             if (inet_pton(AF_INET, "127.0.0.1", &(server_addr.sin_addr)) <= 0) {
    //                 perror("inet_pton");
    //                 exit(EXIT_FAILURE);
    //             }

    //             if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    //                 perror("connect");
    //                 exit(EXIT_FAILURE);
    //             }

    //             output_fd = sockfd;
    //         }
    //     } else if (strcmp(argv[i], "-b") == 0) {
    //         // Input and output to socket
    //         i++;
    //         if (i < argc) {
    //             int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    //             if (sockfd < 0) {
    //                 perror("socket");
    //                 exit(EXIT_FAILURE);
    //             }

    //             struct sockaddr_in server_addr;
    //             memset(&server_addr, 0, sizeof(server_addr));
    //             server_addr.sin_family = AF_INET;
    //             server_addr.sin_port = htons(atoi(argv[i]));
    //             server_addr.sin_addr.s_addr = INADDR_ANY;

    //             if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    //                 perror("bind");
    //                 exit(EXIT_FAILURE);
    //             }

    //             if (listen(sockfd, 1) < 0) {
    //                 perror("listen");
    //                 exit(EXIT_FAILURE);
    //             }

    //             client_fd = accept(sockfd, NULL, NULL);
    //             if (client_fd < 0) {
    //                 perror("accept");
    //                 exit(EXIT_FAILURE);
    //             }

    //             close(sockfd);
    //             input_fd = client_fd;
    //             output_fd = client_fd;
    //         }
    //     } else if (strcmp(argv[i], "-e") == 0) {
    //         // Execute command
    //         i++;
    //         if (i < argc) {
    //             // Create pipes
    //             int pipe_in[2];
    //             int pipe_out[2];
    //             if (pipe(pipe_in) < 0 || pipe(pipe_out) < 0) {
    //                 perror("pipe");
    //                 exit(EXIT_FAILURE);
    //             }

    //             // Fork child process
    //             pid_t pid = fork();
    //             if (pid < 0) {
    //                 perror("fork");
    //                 exit(EXIT_FAILURE);
    //             } else if (pid == 0) {
    //                 // Child process

    //                 // Close unused pipe ends
    //                 close(pipe_in[1]);
    //                 close(pipe_out[0]);

    //                 // Redirect input and output to pipes
    //                 dup2(pipe_in[0], STDIN_FILENO);
    //                 dup2(pipe_out[1], STDOUT_FILENO);

    //                 // Execute command
    //                 execl("/bin/sh", "sh", "-c", argv[i], NULL);

    //                 // If execl fails, exit child process
    //                 perror("execl");
    //                 exit(EXIT_FAILURE);
    //             } else {
    //                 // Parent process

    //                 // Close unused pipe ends
    //                 close(pipe_in[0]);
    //                 close(pipe_out[1]);

    //                 // Redirect input and output to pipes
    //                 dup2(pipe_in[1], input_fd);
    //                 dup2(pipe_out[0], output_fd);

    //                 // Wait for child process to finish
    //                 wait(NULL);
    //             }
    //         }
    //     }
    // }

    // // Close sockets
    // if (server_fd >= 0) {
    //     close(server_fd);
    // }
    // if (client_fd >= 0) {
    //     close(client_fd);
    // }

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, "127.0.0.1", &(server_addr.sin_addr)) <= 0)
    {
        perror("inet_pton");
        exit(EXIT_FAILURE);
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    // for (int i = 1; i <= 9; i++)
    // {
    //     char number = '0' + i;
    //     if (write(sockfd, &number, 1) != 1)
    //     {
    //         perror("write");
    //         exit(EXIT_FAILURE);
    //     }
    //     sleep(1); // wait for a second before sending the next number
    // }
    char number[2];
    while (1)
    {
        printf("Enter a number between 1 and 9: ");
        fflush(stdout);

        if (fgets(number, sizeof(number), stdin) == NULL)
        {
            printf("\nEnd of input\n");
            break;
        }

        // Check if the input is a number between 1 and 9
        if (strlen(number) != 2 || number[0] < '1' || number[0] > '9' || number[1] != '\n')
        {
            printf("Invalid input\n");
            continue;
        }

        if (write(sockfd, number, 1) != 1) // like send, wirting to the socket descriptor
        {
            perror("write");
            exit(EXIT_FAILURE);
        }
    }
    close(sockfd);

    return 0;
}

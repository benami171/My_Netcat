#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    // Redirect input/output to socket
    if (argc >= 4 && strcmp(argv[1], "-e") == 0)
    {
        int pipefd[2];
        pid_t pid;

        if (pipe(pipefd) == -1)
        {
            perror("pipe");
            exit(EXIT_FAILURE);
        }

        pid = fork();
        if (pid == -1)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
            // Child process
            close(pipefd[1]); // Close unused write end of the pipe

            // Redirect input from socket
            if (argc >= 6 && strcmp(argv[3], "-i") == 0)
            {
                int input_socket = socket(AF_INET, SOCK_STREAM, 0);
                struct sockaddr_in input_address;
                memset(&input_address, '0', sizeof(input_address));
                input_address.sin_family = AF_INET;
                input_address.sin_port = htons(atoi(argv[4]));

                if (inet_pton(AF_INET, "127.0.0.1", &input_address.sin_addr) <= 0)
                {
                    perror("inet_pton");
                    exit(EXIT_FAILURE);
                }

                if (connect(input_socket, (struct sockaddr *)&input_address, sizeof(input_address)) < 0)
                {
                    perror("connect");
                    exit(EXIT_FAILURE);
                }

                dup2(input_socket, STDIN_FILENO);
                close(input_socket);
            }

            // Redirect output to socket
            if (argc >= 8 && strcmp(argv[5], "-o") == 0)
            {
                int output_socket = socket(AF_INET, SOCK_STREAM, 0);
                struct sockaddr_in output_address;
                memset(&output_address, '0', sizeof(output_address));
                output_address.sin_family = AF_INET;
                output_address.sin_port = htons(atoi(argv[6]));

                if (inet_pton(AF_INET, "127.0.0.1", &output_address.sin_addr) <= 0)
                {
                    perror("inet_pton");
                    exit(EXIT_FAILURE);
                }

                if (connect(output_socket, (struct sockaddr *)&output_address, sizeof(output_address)) < 0)
                {
                    perror("connect");
                    exit(EXIT_FAILURE);
                }

                dup2(output_socket, STDOUT_FILENO);
                close(output_socket);
            }

            // Execute the command
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);

            execl("/bin/sh", "sh", "-c", argv[2], NULL);
            perror("execl");
            exit(EXIT_FAILURE);
        }
        else
        {
            // Parent process
            close(pipefd[0]); // Close unused read end of the pipe

            // Send the board to the client
            send(new_socket, buffer, strlen(buffer), 0);

            // Read from socket and write to the pipe
            while ((valread = read(new_socket, buffer, sizeof(buffer))) > 0)
            {
                write(pipefd[1], buffer, valread);
            }

            close(pipefd[1]);
            wait(NULL);
        }
    }

    return 0;
}
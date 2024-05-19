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

    char number[3];
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
        if (number[0] < '1' || number[0] > '9' || number[1] != '\n')
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

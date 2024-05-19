// #include <stdio.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <string.h>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <sys/wait.h>

// #define MAX_BUFFER_SIZE 1024

// void executeCommand(char* command, int input, int output) {
//     // Create pipes for input and output
//     int inputPipe[2], outputPipe[2];
//     pipe(inputPipe);
//     pipe(outputPipe);

//     // Fork a child process
//     pid_t pid = fork();

//     if (pid == 0) {
//         // Child process

//         // Close unused ends of the pipes
//         close(inputPipe[1]);
//         close(outputPipe[0]);

//         // Redirect input and output to the pipes
//         dup2(inputPipe[0], STDIN_FILENO);
//         dup2(outputPipe[1], STDOUT_FILENO);

//         // Execute the command
//         system(command);

//         // Close the remaining ends of the pipes
//         close(inputPipe[0]);
//         close(outputPipe[1]);

//         // Exit the child process
//         exit(0);
//     } else if (pid > 0) {
//         // Parent process

//         // Close unused ends of the pipes
//         close(inputPipe[0]);
//         close(outputPipe[1]);

//         // Read input from the socket and write it to the input pipe
//         char buffer[MAX_BUFFER_SIZE];
//         ssize_t bytesRead;
//         while ((bytesRead = read(input, buffer, sizeof(buffer))) > 0) {
//             write(inputPipe[1], buffer, bytesRead);
//         }

//         // Close the input pipe
//         close(inputPipe[1]);

//         // Read output from the output pipe and write it to the socket
//         while ((bytesRead = read(outputPipe[0], buffer, sizeof(buffer))) > 0) {
//             write(output, buffer, bytesRead);
//         }

//         // Close the output pipe
//         close(outputPipe[0]);

//         // Wait for the child process to exit
//         wait(NULL);
//     }
// }

// int main(int argc, char* argv[]) {
//     if (argc < 4) {
//         printf("Usage: %s -e <command> [-i <input_socket>] [-o <output_socket>]\n", argv[0]);
//         return 1;
//     }

//     char* command = NULL;
//     char* inputSocket = NULL;
//     char* outputSocket = NULL;

//     // Parse command line arguments
//     for (int i = 1; i < argc; i++) {
//         if (strcmp(argv[i], "-e") == 0) {
//             if (i + 1 < argc) {
//                 command = argv[i + 1];
//                 i++;
//             }
//         } else if (strcmp(argv[i], "-i") == 0) {
//             if (i + 1 < argc) {
//                 inputSocket = argv[i + 1];
//                 i++;
//             }
//         } else if (strcmp(argv[i], "-o") == 0) {
//             if (i + 1 < argc) {
//                 outputSocket = argv[i + 1];
//                 i++;
//             }
//         }
//     }

//     // Create input socket
//     int inputSocketFD = -1;
//     if (inputSocket != NULL) {
//         inputSocketFD = socket(AF_INET, SOCK_STREAM, 0);
//         if (inputSocketFD < 0) {
//             perror("Failed to create input socket");
//             return 1;
//         }

//         struct sockaddr_in inputAddr;
//         memset(&inputAddr, 0, sizeof(inputAddr));
//         inputAddr.sin_family = AF_INET;
//         inputAddr.sin_port = htons(atoi(inputSocket + 3));
//         inputAddr.sin_addr.s_addr = INADDR_ANY;

//         if (bind(inputSocketFD, (struct sockaddr*)&inputAddr, sizeof(inputAddr)) < 0) {
//             perror("Failed to bind input socket");
//             return 1;
//         }

//         if (listen(inputSocketFD, 1) < 0) {
//             perror("Failed to listen on input socket");
//             return 1;
//         }
//     }

//     // Create output socket
//     int outputSocketFD = -1;
//     if (outputSocket != NULL) {
//         outputSocketFD = socket(AF_INET, SOCK_STREAM, 0);
//         if (outputSocketFD < 0) {
//             perror("Failed to create output socket");
//             return 1;
//         }

//         struct sockaddr_in outputAddr;
//         memset(&outputAddr, 0, sizeof(outputAddr));
//         outputAddr.sin_family = AF_INET;
//         outputAddr.sin_port = htons(atoi(outputSocket + 3));
//         if (inet_pton(AF_INET, "127.0.0.1", &(outputAddr.sin_addr)) <= 0) {
//             perror("Failed to convert output socket address");
//             return 1;
//         }

//         if (connect(outputSocketFD, (struct sockaddr*)&outputAddr, sizeof(outputAddr)) < 0) {
//             perror("Failed to connect to output socket");
//             return 1;
//         }
//     }

//     // Execute the command
//     if (inputSocketFD != -1 && outputSocketFD != -1) {
//         executeCommand(command, inputSocketFD, outputSocketFD);
//     } else if (inputSocketFD != -1) {
//         executeCommand(command, inputSocketFD, STDOUT_FILENO);
//     } else if (outputSocketFD != -1) {
//         executeCommand(command, STDIN_FILENO, outputSocketFD);
//     } else {
//         executeCommand(command, STDIN_FILENO, STDOUT_FILENO);
//     }

//     // Close sockets
//     if (inputSocketFD != -1) {
//         close(inputSocketFD);
//     }
//     if (outputSocketFD != -1) {
//         close(outputSocketFD);
//     }

//     return 0;
// }


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>

int main(int argc, char *argv[])
{
    if (argc != 3 || strcmp(argv[1], "-e") != 0){
        printf("Usage: %s -e \"<program> <arguments>\"\n", argv[0]);
        return 1;
    }

    // Split the second argument into program name and arguments
    char *program = strtok(argv[2], " ");
    if (program == NULL) {
        fprintf(stderr, "Error: No program specified\n");
        return 1;
    }

    // Count the number of arguments
    int arg_count = 0;
    char *arg = program;
    while (arg != NULL) {
        arg_count++;
        arg = strtok(NULL, " ");
    }

    // Allocate memory for arguments array
    char **new_argv = malloc((arg_count + 1) * sizeof(char *));
    if (new_argv == NULL) {
        perror("malloc");
        return 1;
    }

    // Split the arguments again and populate the new_argv array
    new_argv[0] = program;
    arg = strtok(argv[2], " ");
    for (int i = 1; i < arg_count; i++) {
        new_argv[i] = strtok(NULL, " ");
    }
    new_argv[arg_count] = NULL;

    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        free(new_argv);
        return 1;
    }
    else if (pid == 0)
    {
        // Child process
        execvp(program, new_argv);

        // If execvp returns, there was an error
        perror("execvp");
        free(new_argv);
        return 1;
    }
    else
    {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
    }

    free(new_argv);
    return 0;
}
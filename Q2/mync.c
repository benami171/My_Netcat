// #include <stdio.h>
// #include <unistd.h>
// #include <string.h>

// int main(int argc, char *argv[])
// {
//     if (argc != 4 || strcmp(argv[1], "-e") != 0)
//     {
//         printf("Usage: %s -e <program>\n", argv[0]);
//         return 1;
//     }

//     char *new_argv[] = {argv[2], argv[3], NULL};
//     execv(argv[2], new_argv);

//     // If execv returns, there was an error
//     perror("execv");
//     return 1;
// }

// #include <stdio.h>
// #include <unistd.h>
// #include <string.h>
// #include <sys/wait.h>

// int main(int argc, char *argv[])
// {
//     if (argc != 4 || strcmp(argv[1], "-e") != 0)
//     {
//         printf("Usage: %s -e <program>\n", argv[0]);
//         return 1;
//     }

//     char *new_argv[] = {argv[2], argv[3], NULL};

//     pid_t pid = fork();

//     if (pid < 0)
//     {
//         perror("fork");
//         return 1;
//     }
//     else if (pid == 0)
//     {
//         // Child process
//         execv(argv[2], new_argv);

//         // If execv returns, there was an error
//         perror("execv");
//         return 1;
//     }
//     else
//     {
//         // Parent process
//         int status;
//         waitpid(pid, &status, 0);
//     }

//     return 0;
// }
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    if (argc != 3 || strcmp(argv[1], "-e") != 0)
    {
        printf("Usage: %s -e \"<program> <arguments>\"\n", argv[0]);
        return 1;
    }

    // Split the second argument into program name and arguments
    char *program = strtok(argv[2], " ");
    char *arguments = strtok(NULL, "");

    char *new_argv[] = {program, arguments, NULL};

    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return 1;
    }
    else if (pid == 0)
    {
        // Child process
        execvp(program, new_argv);

        // If execvp returns, there was an error
        perror("execvp");
        return 1;
    }
    else
    {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
    }

    return 0;
}
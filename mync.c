#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[])
{
    if (argc != 3 || strcmp(argv[1], "-e") != 0)
    {
        printf("Usage: %s -e <program>\n", argv[0]);
        return 1;
    }

    char *new_argv[] = {argv[2], NULL};
    execv(argv[2], new_argv);

    // If execv returns, there was an error
    perror("execv");
    return 1;
}
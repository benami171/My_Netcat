#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 3

// Function to check if a player has won
int check_winner(int board[SIZE][SIZE])
{
    // Check rows
    for (int i = 0; i < SIZE; i++)
    {
        if (board[i][0] != 0 && board[i][0] == board[i][1] && board[i][1] == board[i][2])
        {
            return board[i][0];
        }
    }

    // Check columns
    for (int i = 0; i < SIZE; i++)
    {
        if (board[0][i] != 0 && board[0][i] == board[1][i] && board[1][i] == board[2][i])
        {
            return board[0][i];
        }
    }

    // Check diagonals
    if (board[0][0] != 0 && board[0][0] == board[1][1] && board[1][1] == board[2][2])
    {
        return board[0][0];
    }
    if (board[0][2] != 0 && board[0][2] == board[1][1] && board[1][1] == board[2][0])
    {
        return board[0][2];
    }

    // No winner
    return 0;
}

// Function to print the current board
void print_board(int board[SIZE][SIZE])
{
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            if (board[i][j] == 1) // computer
            {
                printf("X ");
                fflush(stdin);
            }
            else if (board[i][j] == -1) // player
            {
                printf("O ");
                fflush(stdin);
            }
            else
            {
                printf(". ");
                fflush(stdin);
            }
        }
        printf("\n");
    }
}

// Function to check if the board is full
int is_full(int board[SIZE][SIZE])
{
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            if (board[i][j] == 0)
            {
                return 0; // Not full
            }
        }
    }
    return 1; // Full
}

// Function to check the input
int check_input(char *input)
{
    int counts[9] = {0};

    // Check length
    if (strlen(input) != 9)
    {
        return -1; // Error: wrong length
    }

    for (int i = 0; i < 9; i++)
    {
        int num = input[i] - '0';

        // Check if number is between 1 and 9
        if (num < 1 || num > 9)
        {
            return -2; // Error: number not between 1 and 9
        }

        // Check for duplicates
        if (counts[num - 1] > 0)
        {
            return -3; // Error: duplicate number
        }

        counts[num - 1]++;
    }

    // Check if every number between 1 and 9 is present
    for (int i = 0; i < 9; i++)
    {
        if (counts[i] != 1)
        {
            return -4; // Error: not all numbers present
        }
    }

    return 0; // No errors
}
/**
 * void validate_input(const char *input) {
    int counter[9] = {0};
    while (*input) {
        if (*input < '1' || *input > '9') {
            printf("Invalid input - input should be between 1 and 9\n");
            printf("got: %c, ascii: %d\n", *input, *input);
            exit(1);
        }

        counter[*input - '1']++;
        if (counter[*input - '1'] > 1) {
            printf("Invalid input - duplicate number %c\n", *input);
            exit(1);
        }
        input++;
    }

    // check if we have all the numbers
    for (int i = 0; i < 9; i++) {
        if (counter[i] != 1) {
            printf("Invalid input - missing number %d\n", i + 1);
            exit(1);
        }
    }
}

 *
*/
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <number>\n", argv[0]);
        fflush(stdout);
        return 1;
    }

    int error = check_input(argv[1]);
    if (error != 0)
    {
        printf("Error: Invalid input.\n");
        fflush(stdout);
        return error;
    }

    // initoalizing the board
    int board[SIZE][SIZE] = {0};

    // getitng the moves from the input
    int cpu_moves[9];
    for (int i = 0; i < 9; i++)
    {
        cpu_moves[i] = argv[1][i] - '0';
    }

    int turn = 0;
    while (1)
    {
        if (turn % 2 == 0)
        {
            // CPU's turn
            for (int i = 0; i < 9; i++)
            {
                int move = cpu_moves[i];
                int row = (move - 1) / SIZE;
                int col = (move - 1) % SIZE;
                if (board[row][col] == 0)
                {
                    board[row][col] = 1;
                    break;
                }
            }
        }
        else
        {
            // User's turn
            int move;
            char input[10];
            do
            {

                if (fgets(input, sizeof(input), stdin) == NULL)
                {
                    printf("Error reading input. Please try again.\n");
                    fflush(stdout);

                    continue;
                }

                if (input[0] == '\n')
                {
                    printf("Invalid input. Please enter a number between 1 and 9.\n");
                    fflush(stdout);
                    continue;
                }

                if (sscanf(input, "%d", &move) != 1)
                {
                    printf("Invalid input. Please enter a number between 1 and 9.\n");
                    fflush(stdout);
                    continue;
                }
                if (move < 1 || move > 9)
                {
                    printf("Enter a number between 1 and 9\n");
                    fflush(stdout);
                    continue;
                }

                int row = (move - 1) / SIZE;
                int col = (move - 1) % SIZE;
                if (board[row][col] == 0)
                {
                    board[row][col] = -1;
                    break;
                }
                else
                {
                    printf("Cell is already occupied. Please enter a different move.\n");
                }
            } while (1);
        }
        print_board(board);
        printf("\n");
        fflush(stdout);
        turn++;

        int winner = check_winner(board);
        if (winner != 0)
        {
            if (winner == 1)
            {
                // printf("I win\n");
                printf("\033[1;33mI win\033[0m\n");

                fflush(stdout);
            }
            else
            {
                // printf("I lost\n");
                printf("\033[1;34mI lost blue\033[0m\n");
                fflush(stdout);
            }
            return 0;
        }
        // If there's no winner, check if the board is full
        if (is_full(board))
        {
            printf("DRAW\n");
            fflush(stdout);
            return 0;
        }
    }

    return 0;
}

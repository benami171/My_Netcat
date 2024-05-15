#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#define Computer 'X'
#define Human 'O'

// checking that the number is 9 digits long and without any repeating digits
int check_digits(int num)
{
    char str[11];
    sprintf(str, "%d", num);

    int digits[10] = {0};

    for (int i = 0; i < strlen(str); i++)
    {
        int digit = str[i] - '0';
        if (digit == 0 || digits[digit] == 1)
        {
            return 0;
        }
        digits[digit] = 1;
    }

    for (int i = 1; i <= 9; i++)
    {
        if (digits[i] == 0)
        {
            return 0;
        }
    }

    return 1;
}

void init_board(char board[3][3])
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            board[i][j] = 'E';
        }
    }
}

int board_not_full(char board[3][3])
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (board[i][j] == 'E')
            {
                return 1;
            }
        }
    }
    return 0;
}
void print_board(char board[3][3])
{
    for (int i = 0; i < 3; i++)
    {
        printf("-------------\n");
        for (int j = 0; j < 3; j++)
        {
            printf("| %c ", board[i][j]);
        }
        printf("|\n");
    }
    printf("-------------\n");
}

int digit_count(int num)
{
    int temp = num;
    int count = 0;
    while (temp)
    {
        count++;
        temp /= 10;
    }
    return count;
}

// getting the comptuer move starting from the most left digit
int get_computer_move(int num)
{
    int count = digit_count(num);
    // int move= num % 
    int move = num / (int)pow(10, count - 1);
    return move;
}

int check_human_win(char board[3][3])
{
    // checking rows
    for (int i = 0; i < 3; i++)
    {
        if (board[i][0] == Human && board[i][1] == Human && board[i][2] == Human)
        {
            return 1;
        }
    }

    // checking columns
    for (int i = 0; i < 3; i++)
    {
        if (board[0][i] == Human && board[1][i] == Human && board[2][i] == Human)
        {
            return 1;
        }
    }

    // checking diagonals
    if (board[0][0] == Human && board[1][1] == Human && board[2][2] == Human)
    {
        return 1;
    }

    if (board[0][2] == Human && board[1][1] == Human && board[2][0] == Human)
    {
        return 1;
    }

    return 0;
}

int check_computer_win(char board[3][3])
{
    // checking rows
    for (int i = 0; i < 3; i++)
    {
        if (board[i][0] == Computer && board[i][1] == Computer && board[i][2] == Computer)
        {
            return 1;
        }
    }

    // checking columns
    for (int i = 0; i < 3; i++)
    {
        if (board[0][i] == Computer && board[1][i] == Computer && board[2][i] == Computer)
        {
            return 1;
        }
    }

    // checking diagonals
    if (board[0][0] == Computer && board[1][1] == Computer && board[2][2] == Computer)
    {
        return 1;
    }

    if (board[0][2] == Computer && board[1][1] == Computer && board[2][0] == Computer)
    {
        return 1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <number>\n", argv[0]);
        return 1;
    }

    int num = atoi(argv[1]);
    if (!check_digits(num))
    {
        printf("Invalid number\n");
        return 1; // or exit(1);
    }

    int pipefd[2]; // pipefd[0] is for reading and pipefd[1] is for writing
    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        return 1;
    }

    char board[3][3];
    init_board(board);
    print_board(board);

    // The game loop

    while (board_not_full(board))
    {
        printf("Enter player number: ");
        int player;
        if (scanf("%d", &player) != 1)
        {
            printf("Invalid input\n");
            return 1;
        }
        if (player < 1 || player > 9)
        {
            printf("Enter a number between 1 and 9\n");
            scanf("%d", &player);
        }
        if (board[(player - 1) / 3][(player - 1) % 3] != 'E' || board[(player - 1) / 3][(player - 1) % 3] != Computer || board[(player - 1) / 3][(player - 1) % 3] != Human)
        {
            board[(player - 1) / 3][(player - 1) % 3] = Human;
        }

        if (check_human_win(board))
        {
            printf("Human wins\n");
            break;
        }

        print_board(board);

        // getting the computer move
        int computer_move = get_computer_move(num);
        
        printf("Computer move: %d\n", computer_move);
        if (board[(computer_move - 1) / 3][(computer_move - 1) % 3] != 'E' || board[computer_move / 3][computer_move % 3] != Human || board[computer_move / 3][computer_move % 3] != Computer)
        {
            board[(computer_move - 1) / 3][(computer_move - 1) % 3] = Computer;
        }

        check_computer_win(board);
        print_board(board);

        if (check_computer_win(board))
        {
            printf("Computer wins\n");
            break;
        }

        if (!board_not_full(board))
        {
            printf("It's a tie\n");
            break;
        }
    }

    return 0;
}
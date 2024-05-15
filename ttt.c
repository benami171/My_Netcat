#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <number>\n", argv[0]);
        return 1;
    }
    int num = atoi(argv[1]);
    if (check_digits(num))
    {
        printf("The number has all digits from 1 to 9 and each digit appears only once.\n");
    }
    else
    {
        printf("The number does not meet the criteria.\n");
    }
    return 0;
}
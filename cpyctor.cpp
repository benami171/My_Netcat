/******************************************************************************

Welcome to GDB Online.
GDB online is an online compiler and debugger tool for C, C++, Python, Java, PHP, Ruby, Perl,
C#, OCaml, VB, Swift, Pascal, Fortran, Haskell, Objective-C, Assembly, HTML, CSS, JS, SQLite, Prolog.
Code, Compile, Run and Debug online from anywhere in world.

*******************************************************************************/
#include <iostream>
#include <utility>
#include <cstring>

class String
{
    char *str;
    int size = 0;

public:
    String(const char *st) : size(0) // initial list
    {
        size = strlen(st);

        str = new char[size + 1];
        strcpy(str, st);
    };

    String(const String &s)
    {
        this->str = new char[s.getSize() + 1];
        strcpy(str, s.str);
    };

    ~String()
    {
        delete[] str;
    };

    int getSize() const
    {
        return this->size;
    }

    void operator=(const String &s2)
    {
        // *this = String(s2); // with out *, it'll try to put object in an address
        // return *this;
        if (this->size > 0)
        {
            delete[] this->str;
        }

        this->str = new char[s2.getSize() + 1];
        strcpy(this->str, s2.str);
        this->size = s2.getSize();
    }

    friend std::ostream &operator<<(std::ostream &os, const String &s)
    {
        if (s.getSize() > 0)
            return os << s.str << std::endl;
    }
};

int main()
{
    String s1 = ("Hello world");
    String s2 = s1;
    std::cout << s1;
    std::cout << s2;
    return 0;
}

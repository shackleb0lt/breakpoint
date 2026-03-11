#include <stdio.h>

void func1(char *msg, unsigned int x)
{
    printf("%s %x\n", msg, x);
}

int main()
{
    unsigned int x = 0;
    x += 0x12;
    func1("First Run", x);

    x += 0x34;
    func1("Second Run", x);

    x += 0x56;
    func1("Third Run", x);

    return 0;
}
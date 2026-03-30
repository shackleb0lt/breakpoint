/**
 * MIT License
 *
 * Copyright (c) 2025 Aniruddha Kawade
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <signal.h>

void an_innocent_function()
{
    puts("Putting pineapple on pizza...");
}

void an_innocent_function_end() {}

int checksum()
{
    volatile const char* start = (volatile const char*)&an_innocent_function;
    volatile const char* end   = (volatile const char*)&an_innocent_function_end;
    
    int sum = 0;
    while (start != end)
    {
        sum += *start++;
    }
    return sum;
}

int main()
{
    int safe = checksum();

    void *ptr = (void *) &an_innocent_function;
    write(STDOUT_FILENO, &ptr, sizeof(void *));
    fflush(stdout);

    raise(SIGTRAP);

    while (1)
    {
        if (checksum() == safe)
            an_innocent_function();
        else
            puts("Putting pepporoni on pizza...");

        fflush(stdout);
        raise(SIGTRAP);
    }

    return 0;
}
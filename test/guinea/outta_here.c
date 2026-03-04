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

#define STRINGIFY(x) #x
#define XSTR(x) STRINGIFY(x)

#if defined(__aarch64__)
#define __NR_getpid  172
#define __NR_kill    129
#endif

#if defined(__x86_64__)
#define __NR_getpid  39
#define __NR_kill    62
#endif

#define SIGTRAP 5

int main(int argc, char *argv[])
{
    (void) argv;
    if (argc == 1)
        return 0;

#if defined(__aarch64__)
    asm volatile
    (
        /* getpid() → x0 = pid */
        "mov x8, #" XSTR(__NR_getpid)  " \n\t"
        "svc #0                          \n\t"

        /* kill(pid, SIGTRAP=5) */
        "mov x1, #" XSTR(SIGTRAP)      " \n\t"
        "mov x8, #" XSTR(__NR_kill)    " \n\t"
        "svc #0                          \n\t"
        :   /* no outputs */
        :   /* no inputs  */
        :   /* clobbers   */
            "x0",  "x1",  "x8",  "x9"
    );
#endif

#if defined(__x86_64__)
    asm volatile
    (
        /* getpid() → rax = pid */
        "movq $" XSTR(__NR_getpid)  ", %%rax\n\t"
        "syscall\n\t"

        // Move pid result to rdi (arg 1 for kill)
        "movq %%rax, %%rdi\n\t"

        /* kill(pid, SIGTRAP=5) */
        "movq $" XSTR(__NR_kill)    ", %%rax\n\t"
        "movq $" XSTR(__NR_kill)    ", %%rsi\n\t"
        "syscall\n\t"
        : /* no outputs */
        : /* no inputs  */
        : /* clobbers   */
            "rax", "rdi", "rsi"
    );
#endif

    return 0;
}
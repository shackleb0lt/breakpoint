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
#include <stdlib.h>
#include <stdint.h>

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage: %s <64-bit number> <real number>\n", argv[0]);
        printf("Example: %s 0xDEADBEEFCAFEBABE 123.456789\n", argv[0]);
        return 1;
    }

    // Use 'volatile' to prevent the compiler from optimizing these variables away
    volatile uint64_t val64 = strtoull(argv[1], NULL, 16);
    volatile uint32_t val32 = ((uint32_t)(val64 >> 32)) ^ ((uint32_t)(val64 & 0xFFFFFFFF));
    volatile uint16_t val16 = ((uint16_t)(val32 >> 16)) ^ ((uint16_t)(val32 & 0xFFFF));
    volatile uint8_t  val8  = ((uint8_t)(val16 >> 8)) ^ ((uint8_t)(val16 & 0xFF));
    
    volatile double valD = strtod(argv[2], NULL);
    volatile float valF  = (float)valD + (float)val8;

    volatile uint64_t w_val64 = 0;
    volatile uint32_t w_val32 = 0;
    volatile uint16_t w_val16 = 0;
    volatile uint8_t w_val8 = 0;

    volatile double w_valD = 0;
    volatile float w_valF = 0;

    asm volatile(
        // The compiler replaces %0, %1, etc., with memory addresses like [rsp+8]
        "movq %6, %%r12\n\t"      // Load val64 from memory into r12
        "movl %7, %%r13d\n\t"     // Load val32 from memory into r13d
        "movw %8, %%r14w\n\t"     // Load val16 from memory into r14w
        "movb %9, %%bh\n\t"       // Load val8  from memory into bh

        "movsd %10, %%xmm2\n\t"
        "movss %11, %%xmm3\n\t"

        // Syscall: getpid (39)
        "movq $39, %%rax\n\t"
        "syscall\n\t"

        // Move pid result to rdi (arg 1 for kill)
        "movq %%rax, %%rdi\n\t"

        // Syscall: kill (62), SIGTRAP (5)
        "movq $62, %%rax\n\t"
        "movq $5, %%rsi\n\t"
        "syscall\n\t"

        "movq %%r13,  %0\n\t"  // Move uint64 to w_val64
        "movl %%r14d, %1\n\t"  // Move uint32 to w_val32
        "movw %%r12w, %2\n\t"  // Move uint16 to w_val16
        "movb %%dh,   %3\n\t"

        "movsd %%xmm3, %4\n\t"  // Move double value to w_valD
        "movss %%xmm2, %5\n\t"  // Move float value to w_valF
        
        : "=m"(w_val64), "=m"(w_val32), "=m"(w_val16), "=m"(w_val8), "=m"(w_valD), "=m"(w_valF)
        : "m"(val64), "m"(val32), "m"(val16), "m"(val8), "m"(valD), "m"(valF)
        : "rax", "rcx", "r11", "rdi", "rsi",
          "rbx", "rdx", "r12", "r13", "r14", "r11", "xmm2", "xmm3", "memory" 
    );

    if (w_val64 != val64)
    {
        printf("qword %lu %lu", w_val64, val64);
        return 1;
    }

    if (w_val32 != val32)
    {
        printf("dword %u %u\n", w_val32, val32);
        return 1;
    }

    if (w_val16 != val16)
    {
        printf("word %u %u\n", w_val16, val16);
        return 1;
    }

    if (w_val8 != val8)
    {
        printf("byte %u %u\n", w_val8, val8);
        return 1;
    }

    if (w_valD != valD)
    {
        printf("double %a %a\n", w_valD, valD);
        return 1;
    }

    if (w_valF != valF)
    {
        printf("float %a %a\n", w_valF, valF);
        return 1;
    }

    return 0;
}

/*
| Register | Why                                              |
| -------- | ------------------------------------------------ |
| **RAX**  | Return value (PID or error)                      |
| **RCX**  | Overwritten by `syscall` (stores RIP internally) |
| **R11**  | Overwritten (stores RFLAGS internally)           |
| **RDI**  | Used to pass Arg 1                               |
| **RSI**  | Used to pass Arg 2                               |
*/
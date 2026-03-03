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

#define STRINGIFY(x) #x
#define XSTR(x) STRINGIFY(x)

#if defined(__aarch64__)
#define __NR_getpid  172
#define __NR_kill    129
#endif

int write_to_register(char *argv[])
{
    volatile uint64_t val64 = (uint64_t) strtoull(argv[2], NULL, 16);
    volatile uint32_t val32 = (uint32_t) strtoull(argv[3], NULL, 16);
    volatile uint16_t val16 = (uint16_t) strtoull(argv[4], NULL, 16);
    volatile uint8_t  val8  = (uint8_t)  strtoull(argv[5], NULL, 16);

    volatile double valD = strtod(argv[6], NULL);
    volatile float valF  = strtod(argv[7], NULL);

#if defined(__aarch64__)
    asm volatile
    (
        /* ── General-purpose registers ───────────────────────────────── */

        /* x19 = val64  (full 64-bit load) */
        "ldr x19, %[v64]             \n\t"

        /* w20 = val32  (32-bit load; upper 32 bits of x20 zeroed) */
        "ldr w20, %[v32]             \n\t"

        /* ── SIMD / FP registers ──────────────────────────────────────── */

        /* d19 = valD   (64-bit double) */
        "ldr d19,  %[vD]             \n\t"

        /* s20 = valF   (32-bit float) */
        "ldr s20,  %[vF]             \n\t"

        /* b21 = val8   (8-bit, loaded via a GP scratch reg → fmov) */
        "ldrb w9,  %[v8]             \n\t"
        "fmov s21, w9                \n\t"  /* s21[7:0] = val8; upper bits 0  */

        /* h22 = val16  (16-bit, loaded via a GP scratch reg → fmov) */
        "ldrh w9,  %[v16]            \n\t"
        "fmov s22, w9                \n\t"  /* s22[15:0] = val16              */

        /* s23 = val32  (reuse the already-loaded w20) */
        "fmov s23, w20               \n\t"

        /* d24 = val64  (reuse the already-loaded x19) */
        "fmov d24, x19               \n\t"

        /* ── Trap via SIGTRAP ─────────────────────────────────────────── */
        // syscall number in x8
        // max 7 arguments in x0 ... x6 
        // result returned in x0

        /* getpid() → x0 = pid */
        "mov x8, #" XSTR(__NR_getpid) " \n\t"
        "svc #0                         \n\t"

        /* kill(pid, SIGTRAP=5) */
        "mov x1, #5                    \n\t"
        "mov x8, #" XSTR(__NR_kill) "  \n\t"
        "svc #0                        \n\t"

        :   /* no outputs */
        :   [v64] "m" (val64),
            [v32] "m" (val32),
            [v16] "m" (val16),
            [v8]  "m" (val8),
            [vD]  "m" (valD),
            [vF]  "m" (valF)
        :   /* clobbers */
            "x0",  "x1",  "x8",  "x9",   /* scratch/syscall regs */
            "x19", "x20",                /* GP targets           */
            "d19", "d24",                /* 64-bit FP targets    */
            "s20", "s21", "s22", "s23",  /* 32-bit FP targets    */
            "memory"
    );
#endif
    return 0;
}

int read_from_register(char *argv[])
{
    volatile uint64_t val64 = (uint64_t) strtoull(argv[2], NULL, 16);
    volatile uint32_t val32 = (uint32_t) strtoull(argv[3], NULL, 16);
    volatile uint16_t val16 = (uint16_t) strtoull(argv[4], NULL, 16);
    volatile uint8_t  val8  = (uint8_t)  strtoull(argv[5], NULL, 16);
    volatile double   valD  = strtod(argv[6], NULL);
    volatile float    valF  = strtod(argv[7], NULL);

    volatile uint64_t zero64  = 0;
    volatile uint64_t gpr64  = 0;
    volatile uint32_t gpr32  = 0;

    volatile uint64_t fpr64 = 0;
    volatile uint32_t fpr32 = 0;
    volatile uint16_t fpr16  = 0;
    volatile uint8_t  fpr8   = 0;
    volatile double   fprD   = 0;
    volatile float    fprF   = 0;

    // printf("%X\n", val32);

#if defined(__aarch64__)
    asm volatile
    (
        /* x11 = val64  (full 64-bit load) */
        "ldr x11, %[v64]                 \n\t"

        /* ── Trap first (registers were set by the writer side) ──────── */

        /* getpid() → x0 = pid */
        "mov x8, #" XSTR(__NR_getpid)  " \n\t"
        "svc #0                          \n\t"

        /* kill(pid, SIGTRAP=5) */
        "mov x1, #5                      \n\t"
        "mov x8, #" XSTR(__NR_kill) "    \n\t"
        "svc #0                          \n\t"

        /* ── GP registers → memory ───────────────────────────────────── */

        /* zero64 = x11 */
        "str x11,  %[z64]                \n\t"

        /* gpr64 = x9 */
        "str x9, %[g64]                 \n\t"

        /* gpr32 = w10 */
        "str w10, %[g32]                 \n\t"

        /* ── FP/SIMD registers → memory ──────────────────────────────── */

        /* fprD = d19 */
        "str d19, %[fD]                  \n\t"

        /* fprF = s20 */
        "str s20, %[fF]                  \n\t"

        /* fpr8 = b21  (fmov to GP scratch, then strb) */
        "fmov w9, s21                    \n\t"
        "strb w9, %[f8]                  \n\t"

        /* fpr16 = h22  (fmov to GP scratch, then strh) */
        "fmov w9, s22                    \n\t"
        "strh w9, %[f16]                 \n\t"

        /* fpr32 = s23  (fmov to GP scratch, then str 32-bit) */
        "fmov w9, s23                    \n\t"
        "str  w9, %[f32]                 \n\t"

        /* fpr64 = d24  (fmov to GP scratch, then str 64-bit) */
        "fmov x9, d24                    \n\t"
        "str  x9, %[f64]                 \n\t"

        :   /* outputs — "=m" because we are writing to these locations */
            [z64]  "=m" (zero64),
            [g64]  "=m" (gpr64),
            [g32]  "=m" (gpr32),
            [f16]  "=m" (fpr16),
            [f8]   "=m" (fpr8),
            [fD]   "=m" (fprD),
            [fF]   "=m" (fprF),
            [f32]  "=m" (fpr32),
            [f64]  "=m" (fpr64)

        :   /* inputs */
            [v64]  "m" (val64)
        :   /* clobbers */
            /* scratch / syscall regs */
            "x0", "x1", "x8", "x9", "x10", "x11",
            "memory"
    );

    // printf("%X\n", val32);

    if (zero64 != (uint64_t) val32) {
        printf("high x21 not cleared %lX %lX\n", zero64, (uint64_t) val32);
        return 1;
    }
    if (gpr64 != val64) {
        printf("x19 (qword) %lX %lX\n", (unsigned long)gpr64, (unsigned long)val64);
        return 1;
    }
    if (gpr32 != val32) {
        printf("w20 (dword) %X %X\n", gpr32, val32);
        return 1;
    }
    if (fpr64 != val64) {
        printf("x19 (qword) %lX %lX\n", (unsigned long)fpr64, (unsigned long)val64);
        return 1;
    }
    if (fpr32 != val32) {
        printf("w20 (dword) %X %X\n", fpr32, val32);
        return 1;
    }
    if (fpr16 != val16) {
        printf("h22 (word) %X %X\n", fpr16, val16);
        return 1;
    }
    if (fpr8 != val8) {
        printf("b21 (byte) %X %X\n", fpr8, val8);
        return 1;
    }
    if (fprD != valD) {
        printf("d19 (double) %a %a\n", fprD, valD);
        return 1;
    }
    if (fprF != valF) {
        printf("s20 (float) %a %a\n", fprF, valF);
        return 1;
    }
#endif
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc < 8)
    {
        goto print_error;
    }

    if (argv[1][0] == 'r')
        return write_to_register(argv);
    else if (argv[1][0] == 'w')
        return read_from_register(argv);

print_error:
    printf("Usage: %s <r/w> <u64> <u32> <u16> <u8> <dbl> <flt>\n", argv[0]);
    printf("'r' for testing read, 'w' for write, pass hex values for precision\n");

    return 1;
}

/*
Caller (A)                    Callee (B)
──────────────────────────────────────────
calls B                       is called by A
                              does its work
                              returns to A
A resumes...

Caller-saved (x0–x17)

If A wants these values to survive the call to B, A must save them before calling B
B is free to trash them without saving/restoring
Compiler never relies on these holding C variables across a function call

Callee-saved (x19–x28)

B must save and restore these if it uses them
A can rely on these being intact after B returns
Compiler loves these for variables that need to stay alive across multiple function calls
*/
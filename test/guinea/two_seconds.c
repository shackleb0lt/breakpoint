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

#define BLOCK_SIZE 512

#ifdef __aarch64__

static inline unsigned long long
get_tick_frequency(void)
{
    unsigned long long freq;
    __asm__ volatile (
        "mrs %0, cntfrq_el0" 
        : "=r"(freq)
    );
    return freq;
}

static inline unsigned long long
get_ticks_serialized(void)
{
    unsigned long long val;
    // 'isb' acts as the serialization barrier (like cpuid on x86)
    // 'mrs' reads the system register CNTVCT_EL0 into a general purpose register
    __asm__ volatile (
        "isb\n\t"
        "mrs %0, cntvct_el0\n\t"
        "isb\n\t"
        : "=r"(val)
    );
    return val;
}

int main()
{
    int i = 0;
    unsigned long long block[BLOCK_SIZE] = {0};
    unsigned long long freq = get_tick_frequency();
    unsigned long long start = get_ticks_serialized();
    unsigned long long end = start;
    unsigned long long wait_ticks = freq * 2;

    while (end - start < wait_ticks)
    {
        for (i = 1; i < BLOCK_SIZE; i++)
        {
            block[i] = end + block[i - 1];
        }
        end = get_ticks_serialized();
    }

    return 0;
}

#else // x86

static inline unsigned long long
rdtsc_serialized(void)
{
    unsigned int lo, hi;
    __asm__ volatile (
        "cpuid\n\t"
        "rdtsc\n\t"
        : "=a"(lo), "=d"(hi)
        : "a"(0)
        : "rbx", "rcx"
    );
    return ((unsigned long long)hi << 32) | lo;
}

int main()
{
    int i = 0;
    unsigned long long block[BLOCK_SIZE];

    unsigned long long start = rdtsc_serialized();
    unsigned long long end = rdtsc_serialized();

    // With trial and error this runs for 2 seconds
    while (end - start < 5500000000ULL)
    {
        for (i = 1; i < BLOCK_SIZE; i++)
        {
            block[i] = end + block[i - 1];
        }
        end = rdtsc_serialized();
    }

    return 0;
}

#endif
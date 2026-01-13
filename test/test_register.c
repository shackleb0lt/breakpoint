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

#include "test_common.h"
#include <stdint.h>
#include <time.h>

int main()
{
    variant_t var;
    process_t debug_proc = {0};
    srand(time(NULL));

    uint64_t val64 = (((uint64_t)rand()) << 32) | (((uint64_t)rand()));
    uint32_t val32 = ((uint32_t)(val64 >> 32)) ^ ((uint32_t)(val64 & 0xFFFFFFFF));
    uint16_t val16 = ((uint16_t)(val32 >> 16)) ^ ((uint16_t)(val32 & 0xFFFF));
    uint8_t  val8  = ((uint8_t)(val16 >> 8)) ^ ((uint8_t)(val16 & 0xFF));

    double valD = ((double)val32) + ((double) rand() / (double) RAND_MAX);
    float valF = (float)valD + (float)val8;

    unsigned char ret = (uint8_t) rand();
    char val_str[64] = {0};
    char valD_str[64] = {0};
    char *const exec[] = 
    {
        "asm_magic",
        val_str,
        valD_str,
        NULL
    };

    snprintf(val_str, 63, "%lX", val64);
    snprintf(valD_str, 63, "%a", valD);

    assert(launch_proc(&debug_proc, exec) == 0);
    assert(process_exists(debug_proc.pid));
    assert(process_status(debug_proc.pid) == 't');
    assert(debug_proc.kill_on_end == true);
    assert(debug_proc.state == PROC_STOPPED);

    // Resume process execution
    assert(resume_proc(&debug_proc) == 0);
    assert(wait_proc(&debug_proc, NULL) == 0);
    assert(process_status(debug_proc.pid) == 't');
    assert(debug_proc.state == PROC_STOPPED);

    assert(get_register_by_name(&debug_proc, "r12", &var) == 0);
    printf("%lX %lX\n", var.value.u64, val64);
    assert(var.value.u64 == val64);

    assert(get_register_by_name(&debug_proc, "r13d", &var) == 0);
    printf("%X %X\n", var.value.u32, val32);
    assert(var.value.u32 == val32);

    assert(get_register_by_name(&debug_proc, "r14w", &var) == 0);
    printf("%X %X\n", var.value.u16, val16);
    assert(var.value.u16 == val16);

    assert(get_register_by_name(&debug_proc, "bh", &var) == 0);
    printf("%X %X\n", var.value.u8, val8);
    assert(var.value.u8 == val8);

    assert(get_register_by_name(&debug_proc, "xmm2", &var) == 0);
    printf("%a %a\n", var.value.dbl, valD);
    assert(var.value.dbl == valD);

    assert(get_register_by_name(&debug_proc, "xmm3", &var) == 0);
    printf("%a %a\n", var.value.flt, valF);
    assert(var.value.flt == valF);
    
    var.type = TYPE_UINT64; var.value.u64 = val64;
    assert(set_register_by_name(&debug_proc, "r13",  &var) == 0);

    var.type = TYPE_UINT32; var.value.u32 = val32;
    assert(set_register_by_name(&debug_proc, "r14d", &var) == 0);

    var.type = TYPE_UINT16; var.value.u16 = val16;
    assert(set_register_by_name(&debug_proc, "r12w", &var) == 0);
    
    var.type = TYPE_UINT8; var.value.u8 = val8;
    assert(set_register_by_name(&debug_proc, "dh",   &var) == 0);

    var.type = TYPE_DOUBLE; var.value.dbl = valD;
    assert(set_register_by_name(&debug_proc, "xmm3", &var) == 0);

    var.type = TYPE_FLOAT; var.value.flt = valF;
    assert(set_register_by_name(&debug_proc, "xmm2", &var) == 0);

    assert(resume_proc(&debug_proc) == 0);
    assert(wait_proc(&debug_proc, &ret) == 0);

    assert(debug_proc.state == PROC_EXITED);
    assert(ret == 0);

    cleanup_proc(&debug_proc);
    assert(process_exists(debug_proc.pid) == false);
    
    return 0;
}
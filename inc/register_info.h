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

#ifndef BKPT_LIB_REGISTER_INFO_H
#define BKPT_LIB_REGISTER_INFO_H

#include <stdint.h>

typedef enum 
{
    TYPE_UINT8,
    TYPE_UINT16,
    TYPE_UINT32,
    TYPE_UINT64,

    TYPE_FLOAT,
    TYPE_DOUBLE,
    TYPE_LONG_DOUBLE,

    TYPE_VECTOR,
} variant_type_t;

typedef struct
{
    variant_type_t type;
    union
    {
        uint8_t  u8;
        uint16_t u16;
        uint32_t u32;
        uint64_t u64;

        float flt;
        double dbl;
        long double l_dbl;

        uint8_t vector[16];
    } value;
} variant_t;

typedef enum
{
    REG00_INV = 0,
    REG64_RAX,
    REG64_RDX,
    REG64_RCX,
    REG64_RBX,
    REG64_RSI,
    REG64_RDI,
    REG64_RBP,
    REG64_RSP,
    REG64_R8,
    REG64_R9,
    REG64_R10,
    REG64_R11,
    REG64_R12,
    REG64_R13,
    REG64_R14,
    REG64_R15,
    REG64_RIP,
    REG64_EFLAGS,
    REG64_CS,
    REG64_FS,
    REG64_GS,
    REG64_SS,
    REG64_DS,
    REG64_ES,
    REG64_ORIG_RAX,

    REG32_EAX,
    REG32_EBX,
    REG32_ECX,
    REG32_EDX,
    REG32_ESI,
    REG32_EDI,
    REG32_EBP,
    REG32_ESP,

    REG32_R8D,
    REG32_R9D,
    REG32_R10D,
    REG32_R11D,
    REG32_R12D,
    REG32_R13D,
    REG32_R14D,
    REG32_R15D,

    REG16_AX,
    REG16_BX,
    REG16_CX,
    REG16_DX,
    REG16_SI,
    REG16_DI,
    REG16_BP,
    REG16_SP,
    REG16_R8W,
    REG16_R9W,
    REG16_R10W,
    REG16_R11W,
    REG16_R12W,
    REG16_R13W,
    REG16_R14W,
    REG16_R15W,

    REG8H_AH,
    REG8H_BH,
    REG8H_CH,
    REG8H_DH,

    REG8L_AL,
    REG8L_BL,
    REG8L_CL,
    REG8L_DL,
    REG8L_SIL,
    REG8L_DIL,
    REG8L_BPL,
    REG8L_SPL,
    REG8L_R8B,
    REG8L_R9B,
    REG8L_R10B,
    REG8L_R11B,
    REG8L_R12B,
    REG8L_R13B,
    REG8L_R14B,
    REG8L_R15B,

    REG64_DR0,
    REG64_DR1,
    REG64_DR2,
    REG64_DR3,
    REG64_DR4,
    REG64_DR5,
    REG64_DR6,
    REG64_DR7,

    REGFP_CWD,
    REGFP_SWD,
    REGFP_FTW,
    REGFP_FOP,
    REGFP_FRIP,
    REGFP_FRDP,
    REGFP_MXCSR,
    REGFP_MXCR_MASK,

    REGFP_ST0,
    REGFP_ST1,
    REGFP_ST2,
    REGFP_ST3,
    REGFP_ST4,
    REGFP_ST5,
    REGFP_ST6,
    REGFP_ST7,

    REGFP_MM0,
    REGFP_MM1,
    REGFP_MM2,
    REGFP_MM3,
    REGFP_MM4,
    REGFP_MM5,
    REGFP_MM6,
    REGFP_MM7,

    REGFP_XMM0,
    REGFP_XMM1,
    REGFP_XMM2,
    REGFP_XMM3,
    REGFP_XMM4,
    REGFP_XMM5,
    REGFP_XMM6,
    REGFP_XMM7,
    REGFP_XMM8,
    REGFP_XMM9,
    REGFP_XMM10,
    REGFP_XMM11,
    REGFP_XMM12,
    REGFP_XMM13,
    REGFP_XMM14,
    REGFP_XMM15
} register_id;

register_id get_register_id_by_name(const char *name);
const char *get_register_name_by_id(register_id id);

#endif

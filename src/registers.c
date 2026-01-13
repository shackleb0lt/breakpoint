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

#include "registers.h"
#include <string.h>

const register_info_t g_register_info[] =
{
    {REG64_RAX, "rax", 0, 8, GPR_OFFSET(rax), TYPE_UINT64},
    {REG64_RBX, "rbx", 1, 8, GPR_OFFSET(rbx), TYPE_UINT64},
    {REG64_RCX, "rcx", 2, 8, GPR_OFFSET(rcx), TYPE_UINT64},
    {REG64_RDX, "rdx", 3, 8, GPR_OFFSET(rdx), TYPE_UINT64},
    {REG64_RSI, "rsi", 4, 8, GPR_OFFSET(rsi), TYPE_UINT64},
    {REG64_RDI, "rdi", 5, 8, GPR_OFFSET(rdi), TYPE_UINT64},
    {REG64_RBP, "rbp", 6, 8, GPR_OFFSET(rbp), TYPE_UINT64},
    {REG64_RSP, "rsp", 7, 8, GPR_OFFSET(rsp), TYPE_UINT64},
    {REG64_R8,  "r8",  8, 8, GPR_OFFSET(r8),  TYPE_UINT64},
    {REG64_R9,  "r9",  9, 8, GPR_OFFSET(r9),  TYPE_UINT64},
    {REG64_R10, "r10", 10, 8, GPR_OFFSET(r10), TYPE_UINT64},
    {REG64_R11, "r11", 11, 8, GPR_OFFSET(r11), TYPE_UINT64},
    {REG64_R12, "r12", 12, 8, GPR_OFFSET(r12), TYPE_UINT64},
    {REG64_R13, "r13", 13, 8, GPR_OFFSET(r13), TYPE_UINT64},
    {REG64_R14, "r14", 14, 8, GPR_OFFSET(r14), TYPE_UINT64},
    {REG64_R15, "r15", 15, 8, GPR_OFFSET(r15), TYPE_UINT64},
    {REG64_RIP, "rip", 16, 8, GPR_OFFSET(rip), TYPE_UINT64},
    {REG64_EFLAGS, "eflags", 49, 8, GPR_OFFSET(eflags), TYPE_UINT64},
    {REG64_CS, "cs", 51, 8, GPR_OFFSET(cs), TYPE_UINT64},
    {REG64_FS, "fs", 54, 8, GPR_OFFSET(fs), TYPE_UINT64},
    {REG64_GS, "gs", 55, 8, GPR_OFFSET(gs), TYPE_UINT64},
    {REG64_SS, "ss", 52, 8, GPR_OFFSET(ss), TYPE_UINT64},
    {REG64_DS, "ds", 53, 8, GPR_OFFSET(ds), TYPE_UINT64},
    {REG64_ES, "es", 50, 8, GPR_OFFSET(es), TYPE_UINT64},
    {REG64_ORIG_RAX, "orig_rax", -1, 8, GPR_OFFSET(orig_rax), TYPE_UINT64},

    {REG32_EAX,  "eax",  -1, 4, GPR_OFFSET(rax), TYPE_UINT32},
    {REG32_EBX,  "ebx",  -1, 4, GPR_OFFSET(rbx), TYPE_UINT32},
    {REG32_ECX,  "ecx",  -1, 4, GPR_OFFSET(rcx), TYPE_UINT32},
    {REG32_EDX,  "edx",  -1, 4, GPR_OFFSET(rdx), TYPE_UINT32},
    {REG32_ESI,  "esi",  -1, 4, GPR_OFFSET(rsi), TYPE_UINT32},
    {REG32_EDI,  "edi",  -1, 4, GPR_OFFSET(rdi), TYPE_UINT32},
    {REG32_EBP,  "ebp",  -1, 4, GPR_OFFSET(rbp), TYPE_UINT32},
    {REG32_ESP,  "esp",  -1, 4, GPR_OFFSET(rsp), TYPE_UINT32},
    {REG32_R8D,  "r8d",  -1, 4, GPR_OFFSET(r8),  TYPE_UINT32},
    {REG32_R9D,  "r9d",  -1, 4, GPR_OFFSET(r9),  TYPE_UINT32},
    {REG32_R10D, "r10d", -1, 4, GPR_OFFSET(r10), TYPE_UINT32},
    {REG32_R11D, "r11d", -1, 4, GPR_OFFSET(r11), TYPE_UINT32},
    {REG32_R12D, "r12d", -1, 4, GPR_OFFSET(r12), TYPE_UINT32},
    {REG32_R13D, "r13d", -1, 4, GPR_OFFSET(r13), TYPE_UINT32},
    {REG32_R14D, "r14d", -1, 4, GPR_OFFSET(r14), TYPE_UINT32},
    {REG32_R15D, "r15d", -1, 4, GPR_OFFSET(r15), TYPE_UINT32},

    {REG16_AX,   "ax",   -1, 2, GPR_OFFSET(rax), TYPE_UINT16},
    {REG16_BX,   "bx",   -1, 2, GPR_OFFSET(rbx), TYPE_UINT16},
    {REG16_CX,   "cx",   -1, 2, GPR_OFFSET(rcx), TYPE_UINT16},
    {REG16_DX,   "dx",   -1, 2, GPR_OFFSET(rdx), TYPE_UINT16},
    {REG16_SI,   "si",   -1, 2, GPR_OFFSET(rsi), TYPE_UINT16},
    {REG16_DI,   "di",   -1, 2, GPR_OFFSET(rdi), TYPE_UINT16},
    {REG16_BP,   "bp",   -1, 2, GPR_OFFSET(rbp), TYPE_UINT16},
    {REG16_SP,   "sp",   -1, 2, GPR_OFFSET(rsp), TYPE_UINT16},
    {REG16_R8W,  "r8w",  -1, 2, GPR_OFFSET(r8),  TYPE_UINT16},
    {REG16_R9W,  "r9w",  -1, 2, GPR_OFFSET(r9),  TYPE_UINT16},
    {REG16_R10W, "r10w", -1, 2, GPR_OFFSET(r10), TYPE_UINT16},
    {REG16_R11W, "r11w", -1, 2, GPR_OFFSET(r11), TYPE_UINT16},
    {REG16_R12W, "r12w", -1, 2, GPR_OFFSET(r12), TYPE_UINT16},
    {REG16_R13W, "r13w", -1, 2, GPR_OFFSET(r13), TYPE_UINT16},
    {REG16_R14W, "r14w", -1, 2, GPR_OFFSET(r14), TYPE_UINT16},
    {REG16_R15W, "r15w", -1, 2, GPR_OFFSET(r15), TYPE_UINT16},

    {REG8H_AH, "ah", -1, 1, GPR_OFFSET(rax) + 1, TYPE_UINT8},
    {REG8H_BH, "bh", -1, 1, GPR_OFFSET(rbx) + 1, TYPE_UINT8},
    {REG8H_CH, "ch", -1, 1, GPR_OFFSET(rcx) + 1, TYPE_UINT8},
    {REG8H_DH, "dh", -1, 1, GPR_OFFSET(rdx) + 1, TYPE_UINT8},

    {REG8L_AL,   "al",   -1, 1, GPR_OFFSET(rax), TYPE_UINT8},
    {REG8L_BL,   "bl",   -1, 1, GPR_OFFSET(rbx), TYPE_UINT8},
    {REG8L_CL,   "cl",   -1, 1, GPR_OFFSET(rcx), TYPE_UINT8},
    {REG8L_DL,   "dl",   -1, 1, GPR_OFFSET(rdx), TYPE_UINT8},
    {REG8L_SIL,  "sil",  -1, 1, GPR_OFFSET(rsi), TYPE_UINT8},
    {REG8L_DIL,  "dil",  -1, 1, GPR_OFFSET(rdi), TYPE_UINT8},
    {REG8L_BPL,  "bpl",  -1, 1, GPR_OFFSET(rbp), TYPE_UINT8},
    {REG8L_SPL,  "spl",  -1, 1, GPR_OFFSET(rsp), TYPE_UINT8},
    {REG8L_R8B,  "r8b",  -1, 1, GPR_OFFSET(r8),  TYPE_UINT8},
    {REG8L_R9B,  "r9b",  -1, 1, GPR_OFFSET(r9),  TYPE_UINT8},
    {REG8L_R10B, "r10b", -1, 1, GPR_OFFSET(r10), TYPE_UINT8},
    {REG8L_R11B, "r11b", -1, 1, GPR_OFFSET(r11), TYPE_UINT8},
    {REG8L_R12B, "r12b", -1, 1, GPR_OFFSET(r12), TYPE_UINT8},
    {REG8L_R13B, "r13b", -1, 1, GPR_OFFSET(r13), TYPE_UINT8},
    {REG8L_R14B, "r14b", -1, 1, GPR_OFFSET(r14), TYPE_UINT8},
    {REG8L_R15B, "r15b", -1, 1, GPR_OFFSET(r15), TYPE_UINT8},

    {REG64_DR0, "dr0", -1, 8, DR_OFFSET(0), TYPE_UINT64},
    {REG64_DR1, "dr1", -1, 8, DR_OFFSET(1), TYPE_UINT64},
    {REG64_DR2, "dr2", -1, 8, DR_OFFSET(2), TYPE_UINT64},
    {REG64_DR3, "dr3", -1, 8, DR_OFFSET(3), TYPE_UINT64},
    {REG64_DR4, "dr4", -1, 8, DR_OFFSET(4), TYPE_UINT64},
    {REG64_DR5, "dr5", -1, 8, DR_OFFSET(5), TYPE_UINT64},
    {REG64_DR6, "dr6", -1, 8, DR_OFFSET(6), TYPE_UINT64},
    {REG64_DR7, "dr7", -1, 8, DR_OFFSET(7), TYPE_UINT64},

    {REGFP_CWD,  "cwd", 65, 2, FPR_OFFSET(cwd), TYPE_UINT16},
    {REGFP_SWD,  "swd", 66, 2, FPR_OFFSET(swd), TYPE_UINT16},
    {REGFP_FTW,  "ftw", -1, 2, FPR_OFFSET(ftw), TYPE_UINT16},
    {REGFP_FOP,  "fop", -1, 2, FPR_OFFSET(fop), TYPE_UINT16},
    {REGFP_FRIP, "frip",-1, 8, FPR_OFFSET(rip), TYPE_UINT64},
    {REGFP_FRDP, "frdp",-1, 8, FPR_OFFSET(rdp), TYPE_UINT64},
    {REGFP_MXCSR, "mxcsr", 64, 4, FPR_OFFSET(mxcsr), TYPE_UINT32},
    {REGFP_MXCR_MASK, "mxcr_mask", -1, 4, FPR_OFFSET(mxcr_mask), TYPE_UINT32},

    {REGFP_ST0, "st0", 33, 16, STR_OFFSET(0), TYPE_LONG_DOUBLE},
    {REGFP_ST1, "st1", 34, 16, STR_OFFSET(1), TYPE_LONG_DOUBLE},
    {REGFP_ST2, "st2", 35, 16, STR_OFFSET(2), TYPE_LONG_DOUBLE},
    {REGFP_ST3, "st3", 36, 16, STR_OFFSET(3), TYPE_LONG_DOUBLE},
    {REGFP_ST4, "st4", 37, 16, STR_OFFSET(4), TYPE_LONG_DOUBLE},
    {REGFP_ST5, "st5", 38, 16, STR_OFFSET(5), TYPE_LONG_DOUBLE},
    {REGFP_ST6, "st6", 39, 16, STR_OFFSET(6), TYPE_LONG_DOUBLE},
    {REGFP_ST7, "st7", 40, 16, STR_OFFSET(7), TYPE_LONG_DOUBLE},

    {REGFP_MM0, "mm0", 41, 8, STR_OFFSET(0), TYPE_UINT64},
    {REGFP_MM1, "mm1", 42, 8, STR_OFFSET(1), TYPE_UINT64},
    {REGFP_MM2, "mm2", 43, 8, STR_OFFSET(2), TYPE_UINT64},
    {REGFP_MM3, "mm3", 44, 8, STR_OFFSET(3), TYPE_UINT64},
    {REGFP_MM4, "mm4", 45, 8, STR_OFFSET(4), TYPE_UINT64},
    {REGFP_MM5, "mm5", 46, 8, STR_OFFSET(5), TYPE_UINT64},
    {REGFP_MM6, "mm6", 47, 8, STR_OFFSET(6), TYPE_UINT64},
    {REGFP_MM7, "mm7", 48, 8, STR_OFFSET(7), TYPE_UINT64},

    {REGFP_XMM0,  "xmm0",  17, 16, XMM_OFFSET(0),  TYPE_VECTOR},
    {REGFP_XMM1,  "xmm1",  18, 16, XMM_OFFSET(1),  TYPE_VECTOR},
    {REGFP_XMM2,  "xmm2",  19, 16, XMM_OFFSET(2),  TYPE_VECTOR},
    {REGFP_XMM3,  "xmm3",  20, 16, XMM_OFFSET(3),  TYPE_VECTOR},
    {REGFP_XMM4,  "xmm4",  21, 16, XMM_OFFSET(4),  TYPE_VECTOR},
    {REGFP_XMM5,  "xmm5",  22, 16, XMM_OFFSET(5),  TYPE_VECTOR},
    {REGFP_XMM6,  "xmm6",  23, 16, XMM_OFFSET(6),  TYPE_VECTOR},
    {REGFP_XMM7,  "xmm7",  24, 16, XMM_OFFSET(7),  TYPE_VECTOR},
    {REGFP_XMM8,  "xmm8",  25, 16, XMM_OFFSET(8),  TYPE_VECTOR},
    {REGFP_XMM9,  "xmm9",  26, 16, XMM_OFFSET(9),  TYPE_VECTOR},
    {REGFP_XMM10, "xmm10", 27, 16, XMM_OFFSET(10), TYPE_VECTOR},
    {REGFP_XMM11, "xmm11", 28, 16, XMM_OFFSET(11), TYPE_VECTOR},
    {REGFP_XMM12, "xmm12", 29, 16, XMM_OFFSET(12), TYPE_VECTOR},
    {REGFP_XMM13, "xmm13", 30, 16, XMM_OFFSET(13), TYPE_VECTOR},
    {REGFP_XMM14, "xmm14", 31, 16, XMM_OFFSET(14), TYPE_VECTOR},
    {REGFP_XMM15, "xmm15", 32, 16, XMM_OFFSET(15), TYPE_VECTOR}
};

const size_t g_register_info_count =
    sizeof(g_register_info) / sizeof(g_register_info[0]);

const register_info_t *get_register_info_by_name(const char *reg_name)
{
    for (size_t i = 0; i < g_register_info_count; i++)
    {
        if (strncmp(reg_name, g_register_info[i].name, REGNAME_LEN) == 0)
            return (g_register_info + i);
    }
    return NULL;
}

const register_info_t *get_register_info_by_id(register_id id)
{
    for (size_t i = 0; i < g_register_info_count; i++)
    {
        if (g_register_info[i].id == id)
            return (g_register_info + i);
    }

    return NULL;
}

const register_info_t *get_register_info_by_dwarf(int32_t dwarf_id)
{
    if (dwarf_id == -1)
        return NULL;

    for (size_t i = 0; i < g_register_info_count; i++)
    {
        if (g_register_info[i].dwarf_id == dwarf_id)
            return (g_register_info + i);
    }

    return NULL;
}

register_id get_register_id_by_name(const char *name)
{
    const register_info_t *reg_info = get_register_info_by_name(name);    
    if (reg_info == NULL)
        return REG00_INV;

    return reg_info->id;
}

const char *get_register_name_by_id(register_id id)
{
    const register_info_t *reg_info = get_register_info_by_id(id);
    if (reg_info == NULL)
        return "";
    return reg_info->name;
}

size_t get_variant_size(variant_type_t type)
{
    switch (type)
    {
        case TYPE_UINT8:
            return 1;
        case TYPE_UINT16:
            return 2;
        case TYPE_UINT32:
            return 4;
        case TYPE_UINT64:
            return 8;
        case TYPE_FLOAT:
            return 4;
        case TYPE_DOUBLE:
            return 8;
        case TYPE_LONG_DOUBLE:
            return 16;
        case TYPE_VECTOR:
            return 16;
        default:
            return 0;
    }
}


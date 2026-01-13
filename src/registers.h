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

#ifndef BKPT_LIB_REGISTER_H
#define BKPT_LIB_REGISTER_H

#include <stdint.h> 
#include <stddef.h>
#include <sys/user.h>

#include "register_info.h"

#define REGNAME_LEN 12

#define GPR_OFFSET(reg) (offsetof(struct user, regs) + offsetof(struct user_regs_struct, reg))
#define DR_OFFSET(num)  (offsetof(struct user, u_debugreg) + num * 8)
#define FPR_OFFSET(reg) (offsetof(struct user, i387) + offsetof(struct user_fpregs_struct, reg))
#define STR_OFFSET(num) (offsetof(struct user, i387) + offsetof(struct user_fpregs_struct, st_space) + (num*16))
#define XMM_OFFSET(num) (offsetof(struct user, i387) + offsetof(struct user_fpregs_struct, xmm_space) + (num*16))

typedef struct
{
    register_id id;
    char name[REGNAME_LEN];
    int32_t dwarf_id;
    size_t size;
    size_t offset;
    variant_type_t type;
} register_info_t;

extern const register_info_t g_register_info[];
extern const size_t g_register_info_count;

size_t get_variant_size(variant_type_t type);
const register_info_t *get_register_info_by_name(const char *reg_name);
const register_info_t *get_register_info_by_id(register_id id);
const register_info_t *get_register_info_by_dwarf(int32_t dwarf_id);

// PTRACE_GETREGS and PTRACE_SETREGS
// For reading and writing all general-purpose registers at once
// Not usable with x87, MMX, SSE and debug registers

// PTRACE_GETFPREGS and PTRACE_SETFPREGS
// For reading and writing x87, MMX, and SSE registers

// PTRACE_PEEKUSER
// For reading debug registers also works with single general purpose
// 8 bytes at a time

// PTRACE_POKEUSER
// For writing debug registers or a single general-purpose register
// Also 8 bytes at a time

// +----------------------+--------------+----------------+----------------+
// | Register Type        | GET/SET_REGS | GET/SET_FPREGS | PEEK/POKE_USER |
// +----------------------+--------------+----------------+----------------+
// | GPR (RAX, RIP, etc.) | YES          | NO             | YES (Limited)  |
// | FPU (x87, XMM)       | NO           | YES            | NO             |
// | Debug (DR0-DR7)      | NO           | NO             | YES            |
// +----------------------+--------------+----------------+----------------+


// rax Caller-saved general register; used for return values
// rbx Callee-saved general register
// rcx Used to pass the fourth argument to functions
// rdx Used to pass the third argument to functions
// rsp Stack pointer
// rbp Callee-saved general register; optionally used pointer to the top of
// the stack frame
// rsi Used to pass the second argument to functions
// rdi Used to pass the first argument to functions
// r8  Used to pass the fifth argument to functions
// r9  Used to pass the sixth argument to functions
// r1-r11 Caller-saved general registers
// r12-r15 Callee-saved general registers

#endif
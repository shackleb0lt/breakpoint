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

#include "registers.hpp"

#include <stdexcept>

// Macros to keep the table readable and maintainable
#define REG_GPR_X(i) {RegisterID::REG64_X##i, "x"#i, 8, (i * 8), RegisterType::RegGPR}
#define REG_GPR_W(i) {RegisterID::REG32_W##i, "w"#i, 4, (i * 8), RegisterType::RegGPR}


#define REG_VEC_Q(i) {RegisterID::REG128_Q##i, "q"#i, 16, (i * 16), RegisterType::RegFPR}
#define REG_VEC_D(i) {RegisterID::REG64_D##i,  "d"#i, 8,  (i * 16), RegisterType::RegFPR}
#define REG_VEC_S(i) {RegisterID::REG32_S##i,  "s"#i, 4,  (i * 16), RegisterType::RegFPR}
#define REG_VEC_H(i) {RegisterID::REG16_H##i,  "h"#i, 2,  (i * 16), RegisterType::RegFPR}
#define REG_VEC_B(i) {RegisterID::REG8_B##i,   "b"#i, 1,  (i * 16), RegisterType::RegFPR}

const struct RegisterInfo g_register_table[] =
{
    // --- 64-bit General Purpose Registers ---
    REG_GPR_X(0),  REG_GPR_X(1),  REG_GPR_X(2),  REG_GPR_X(3),  REG_GPR_X(4),
    REG_GPR_X(5),  REG_GPR_X(6),  REG_GPR_X(7),  REG_GPR_X(8),  REG_GPR_X(9),
    REG_GPR_X(10), REG_GPR_X(11), REG_GPR_X(12), REG_GPR_X(13), REG_GPR_X(14),
    REG_GPR_X(15), REG_GPR_X(16), REG_GPR_X(17), REG_GPR_X(18), REG_GPR_X(19),
    REG_GPR_X(20), REG_GPR_X(21), REG_GPR_X(22), REG_GPR_X(23), REG_GPR_X(24),
    REG_GPR_X(25), REG_GPR_X(26), REG_GPR_X(27), REG_GPR_X(28), REG_GPR_X(29),
    REG_GPR_X(30),

    // --- 32-bit Partial GPRs (W-registers) ---
    REG_GPR_W(0),  REG_GPR_W(1),  REG_GPR_W(2),  REG_GPR_W(3),  REG_GPR_W(4),
    REG_GPR_W(5),  REG_GPR_W(6),  REG_GPR_W(7),  REG_GPR_W(8),  REG_GPR_W(9),
    REG_GPR_W(10), REG_GPR_W(11), REG_GPR_W(12), REG_GPR_W(13), REG_GPR_W(14),
    REG_GPR_W(15), REG_GPR_W(16), REG_GPR_W(17), REG_GPR_W(18), REG_GPR_W(19),
    REG_GPR_W(20), REG_GPR_W(21), REG_GPR_W(22), REG_GPR_W(23), REG_GPR_W(24),
    REG_GPR_W(25), REG_GPR_W(26), REG_GPR_W(27), REG_GPR_W(28), REG_GPR_W(29),
    REG_GPR_W(30),

    // --- 128-bit Quadword SIMD (Q-registers) ---
    REG_VEC_Q(0),  REG_VEC_Q(1),  REG_VEC_Q(2),  REG_VEC_Q(3),  REG_VEC_Q(4),  REG_VEC_Q(5),  REG_VEC_Q(6),  REG_VEC_Q(7),
    REG_VEC_Q(8),  REG_VEC_Q(9),  REG_VEC_Q(10), REG_VEC_Q(11), REG_VEC_Q(12), REG_VEC_Q(13), REG_VEC_Q(14), REG_VEC_Q(15),
    REG_VEC_Q(16), REG_VEC_Q(17), REG_VEC_Q(18), REG_VEC_Q(19), REG_VEC_Q(20), REG_VEC_Q(21), REG_VEC_Q(22), REG_VEC_Q(23),
    REG_VEC_Q(24), REG_VEC_Q(25), REG_VEC_Q(26), REG_VEC_Q(27), REG_VEC_Q(28), REG_VEC_Q(29), REG_VEC_Q(30), REG_VEC_Q(31),

    // --- 64-bit Doubleword SIMD (D-registers) ---
    REG_VEC_D(0),  REG_VEC_D(1),  REG_VEC_D(2),  REG_VEC_D(3),  REG_VEC_D(4),  REG_VEC_D(5),  REG_VEC_D(6),  REG_VEC_D(7),
    REG_VEC_D(8),  REG_VEC_D(9),  REG_VEC_D(10), REG_VEC_D(11), REG_VEC_D(12), REG_VEC_D(13), REG_VEC_D(14), REG_VEC_D(15),
    REG_VEC_D(16), REG_VEC_D(17), REG_VEC_D(18), REG_VEC_D(19), REG_VEC_D(20), REG_VEC_D(21), REG_VEC_D(22), REG_VEC_D(23),
    REG_VEC_D(24), REG_VEC_D(25), REG_VEC_D(26), REG_VEC_D(27), REG_VEC_D(28), REG_VEC_D(29), REG_VEC_D(30), REG_VEC_D(31),

    // --- 32-bit Singleword SIMD (S-registers) ---
    REG_VEC_S(0),  REG_VEC_S(1),  REG_VEC_S(2),  REG_VEC_S(3),  REG_VEC_S(4),  REG_VEC_S(5),  REG_VEC_S(6),  REG_VEC_S(7),
    REG_VEC_S(8),  REG_VEC_S(9),  REG_VEC_S(10), REG_VEC_S(11), REG_VEC_S(12), REG_VEC_S(13), REG_VEC_S(14), REG_VEC_S(15),
    REG_VEC_S(16), REG_VEC_S(17), REG_VEC_S(18), REG_VEC_S(19), REG_VEC_S(20), REG_VEC_S(21), REG_VEC_S(22), REG_VEC_S(23),
    REG_VEC_S(24), REG_VEC_S(25), REG_VEC_S(26), REG_VEC_S(27), REG_VEC_S(28), REG_VEC_S(29), REG_VEC_S(30), REG_VEC_S(31),

    // --- 16-bit Halfword SIMD (H-registers) ---
    REG_VEC_H(0),  REG_VEC_H(1),  REG_VEC_H(2),  REG_VEC_H(3),  REG_VEC_H(4),  REG_VEC_H(5),  REG_VEC_H(6),  REG_VEC_H(7),
    REG_VEC_H(8),  REG_VEC_H(9),  REG_VEC_H(10), REG_VEC_H(11), REG_VEC_H(12), REG_VEC_H(13), REG_VEC_H(14), REG_VEC_H(15),
    REG_VEC_H(16), REG_VEC_H(17), REG_VEC_H(18), REG_VEC_H(19), REG_VEC_H(20), REG_VEC_H(21), REG_VEC_H(22), REG_VEC_H(23),
    REG_VEC_H(24), REG_VEC_H(25), REG_VEC_H(26), REG_VEC_H(27), REG_VEC_H(28), REG_VEC_H(29), REG_VEC_H(30), REG_VEC_H(31),

    // --- 8-bit Byte SIMD (B-registers) ---
    REG_VEC_B(0),  REG_VEC_B(1),  REG_VEC_B(2),  REG_VEC_B(3),  REG_VEC_B(4),  REG_VEC_B(5),  REG_VEC_B(6),  REG_VEC_B(7),
    REG_VEC_B(8),  REG_VEC_B(9),  REG_VEC_B(10), REG_VEC_B(11), REG_VEC_B(12), REG_VEC_B(13), REG_VEC_B(14), REG_VEC_B(15),
    REG_VEC_B(16), REG_VEC_B(17), REG_VEC_B(18), REG_VEC_B(19), REG_VEC_B(20), REG_VEC_B(21), REG_VEC_B(22), REG_VEC_B(23),
    REG_VEC_B(24), REG_VEC_B(25), REG_VEC_B(26), REG_VEC_B(27), REG_VEC_B(28), REG_VEC_B(29), REG_VEC_B(30), REG_VEC_B(31),

    // --- Special Purpose Registers ---
    {RegisterID::REG64_SP,  "sp",  8, (31 * 8), RegisterType::RegGPR},
    {RegisterID::REG64_PC,  "pc",  8, (32 * 8), RegisterType::RegGPR},
    {RegisterID::REG32_FPSR,  "fpsr",  4, offsetof(struct user_fpsimd_state, fpsr), RegisterType::RegFPR},
    {RegisterID::REG32_FPCR,  "fpsr",  4, offsetof(struct user_fpsimd_state, fpcr), RegisterType::RegFPR}
};

const std::size_t g_register_table_size =
    sizeof(g_register_table)/sizeof(g_register_table[0]);

const RegisterInfo*
get_register_info(std::string_view name)
{
    std::size_t curr = 0;
    for (; curr < g_register_table_size; curr++)
    {
        if (name == g_register_table[curr].name)
            return &g_register_table[curr];
    }

    throw std::invalid_argument("Invalid register name " + std::string(name));
}

const RegisterInfo*
get_register_info(RegisterID id)
{
    std::size_t curr = 0;
    for (; curr < g_register_table_size; curr++)
    {
        if (id == g_register_table[curr].id)
            return &g_register_table[curr];
    }

    throw std::logic_error("Get register with id reached end of function"); 
}


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
#include <cstring>

enum class RegisterType
{
    RegGPR,
    RegFPR,
    Both
};

struct RegisterInfo
{
    RegisterID id;
    std::string_view name;
    std::size_t size;
    std::size_t offset;
    RegisterType type;
};

#if defined(__aarch64__)

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
    {RegisterID::REG32_FPCR,  "fpcr",  4, offsetof(struct user_fpsimd_state, fpcr), RegisterType::RegFPR}
};

const std::size_t g_register_table_size =
    sizeof(g_register_table) / sizeof(g_register_table[0]);

const RegisterInfo *
get_register_info(std::string_view name)
{
    for (std::size_t curr = 0; curr < g_register_table_size; curr++)
    {
        if (name == g_register_table[curr].name)
            return &g_register_table[curr];
    }

    throw std::invalid_argument("Invalid register name " + std::string(name));
}

const RegisterInfo *
get_register_info(RegisterID id)
{
    for (std::size_t curr = 0; curr < g_register_table_size; curr++)
    {
        if (id == g_register_table[curr].id)
            return &g_register_table[curr];
    }

    throw std::logic_error("Register ID not found in table");
}

#elif defined(__x86__)

const RegisterInfo*
get_register_info(std::string_view name)
{
    (void) name;
    throw std::runtime_error("Register access not implemented for x86");
}

const RegisterInfo*
get_register_info(RegisterID id)
{
    (void) id;
    throw std::runtime_error("Register access not implemented for x86")
}
#endif

std::uint8_t *
register_offset(const RegisterInfo *info, Registers& reg_state)
{
    std::uint8_t *offset = nullptr;

    if (info->type == RegisterType::RegGPR)
    {
        offset = static_cast<std::uint8_t *>(reg_state.gpr_ptr());
        return (offset + info->offset);
    }
    else if (info->type == RegisterType::RegFPR)
    {
        offset = static_cast<std::uint8_t *>(reg_state.fpr_ptr());
        return (offset + info->offset);
    }
    throw std::logic_error("Invalide register type in register_offset");
}

RegisterValue
read_commmon(const RegisterInfo *info, std::uint8_t *src_addr)
{
    switch (info->size)
    {
        case 1:
        {
            uint8_t val;
            std::memcpy(&val, src_addr, sizeof(val));
            return val;
        }
        case 2:
        {
            uint16_t val;
            std::memcpy(&val, src_addr, sizeof(val));
            return val;
        }
        case 4:
        {
            uint32_t val;
            std::memcpy(&val, src_addr, sizeof(val));
            return val;
        }
        case 8:
        {
            uint64_t val;
            std::memcpy(&val, src_addr, sizeof(val));
            return val;
        }
        case 16:
        {
            __uint128_t val;
            std::memcpy(&val, src_addr, sizeof(val));
            return val;
        }
        default:
        {
            throw std::logic_error("Register " +
                std::string(info->name) + " has invalid size");
            break;
        }
    }
    
}

RegisterValue
parse_register_token(const RegisterInfo* info, std::string_view token)
{
    if (token.find('.') != std::string_view::npos)
    {
        __uint128_t result = 0;
        
        if (token.back() == 'f' || token.back() == 'F')
        {
            if (info->size < 4)
                throw std::runtime_error("Register too small for float");
            
            float f_val = std::stof(std::string(token.substr(0, token.size() - 1)));
            std::memcpy(&result, &f_val, sizeof(float));
        }
        else
        {
            if (info->size < 8)
                throw std::runtime_error("Register too small for double");
            
            double d_val = std::stod(std::string(token));
            std::memcpy(&result, &d_val, sizeof(double));
        }
        
        if (info->size == 16) return result;
        if (info->size == 8)  return static_cast<uint64_t>(result);
        if (info->size == 4)  return static_cast<uint32_t>(result);
    }

    uint64_t val = 0;
    std::string s_token(token);
    bool is_hex = (token.size() > 2 && token[0] == '0' && (token[1] == 'x' || token[1] == 'X'));

    try
    {
        if (is_hex)
        {
            val = std::stoull(s_token, nullptr, 16);
        }
        else
        {
            val = std::stoull(s_token, nullptr, 10);
        }
    }
    catch (...)
    {
        throw std::runtime_error("Invalid numeric token: " + s_token);
    }

    switch (info->size)
    {
        case 1:
            if (val > UINT8_MAX)
                throw std::runtime_error("Value too large for 8-bit register");
            return static_cast<uint8_t>(val);
        case 2:
            if (val > UINT16_MAX)
                throw std::runtime_error("Value too large for 16-bit register");
            return static_cast<uint16_t>(val);
        case 4:
            if (val > UINT32_MAX)
                throw std::runtime_error("Value too large for 32-bit register");
            return static_cast<uint32_t>(val);
        case 8:
            return val;
        case 16:
            return static_cast<__uint128_t>(val);
        default:
            throw std::runtime_error("Unsupported register size");
    }
}

RegisterValue Registers::read(std::string_view reg_name)
{
    const RegisterInfo *info = get_register_info(reg_name);
    return read_commmon(info, register_offset(info, *this));
}

RegisterValue Registers::read(RegisterID reg_id)
{
    const RegisterInfo *info = get_register_info(reg_id);
    return read_commmon(info, register_offset(info, *this));
}

void Registers::write(std::string_view reg_name, RegisterValue val)
{
    const RegisterInfo *info = get_register_info(reg_name);
    auto func = [&](auto &&arg) -> void
    {
        using T = std::decay_t<decltype(arg)>;
        if (sizeof(T) > info->size)
            throw std::invalid_argument("Size mismatch for register "
            + std::string(info->name));

        std::memcpy(register_offset(info, *this), &arg, sizeof(T));
    };

    std::visit(func, val);
}

void Registers::write(std::string_view reg_name, std::string_view val_str)
{
    const RegisterInfo *info = get_register_info(reg_name);
    RegisterValue val = parse_register_token(info, val_str);
    auto func = [&](auto &&arg) -> void
    {
        using T = std::decay_t<decltype(arg)>;
        if (sizeof(T) > info->size)
            throw std::invalid_argument("Size mismatch for register "
            + std::string(info->name));

        std::memcpy(register_offset(info, *this), &arg, sizeof(T));
    };

    std::visit(func, val);
}

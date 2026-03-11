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

#include <charconv>
#include <cstring>
#include <stdexcept>
#include <string>

enum class ValueType
{
    UInt8,
    UInt16,
    UInt32,
    UInt64,
    UInt128,
    Float,
    Double,
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

#define REG_VEC_V(i) {RegisterID::REG128_V##i, "v"#i, 16, (i * 16), RegisterType::RegFPR}
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
    REG_VEC_V(0),  REG_VEC_V(1),  REG_VEC_V(2),  REG_VEC_V(3),  REG_VEC_V(4),  REG_VEC_V(5),  REG_VEC_V(6),  REG_VEC_V(7),
    REG_VEC_V(8),  REG_VEC_V(9),  REG_VEC_V(10), REG_VEC_V(11), REG_VEC_V(12), REG_VEC_V(13), REG_VEC_V(14), REG_VEC_V(15),
    REG_VEC_V(16), REG_VEC_V(17), REG_VEC_V(18), REG_VEC_V(19), REG_VEC_V(20), REG_VEC_V(21), REG_VEC_V(22), REG_VEC_V(23),
    REG_VEC_V(24), REG_VEC_V(25), REG_VEC_V(26), REG_VEC_V(27), REG_VEC_V(28), REG_VEC_V(29), REG_VEC_V(30), REG_VEC_V(31),

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
    {RegisterID::REG64_PC,      "pc",     8, (32 * 8), RegisterType::RegGPR},
    {RegisterID::REG64_SP,      "sp",     8, (31 * 8), RegisterType::RegGPR},
    {RegisterID::REG64_PSTATE,  "pstate", 8, (33 * 8), RegisterType::RegGPR},
    {RegisterID::REG32_FPSR,    "fpsr",   4, offsetof(struct user_fpsimd_state, fpsr), RegisterType::RegFPR},
    {RegisterID::REG32_FPCR,    "fpcr",   4, offsetof(struct user_fpsimd_state, fpcr), RegisterType::RegFPR}
};

const std::size_t g_register_table_size =
    sizeof(g_register_table) / sizeof(g_register_table[0]);

const RegisterInfo *get_register_info(RegisterID key)
{
    std::size_t index = static_cast<std::size_t>(key);
    if (index < g_register_table_size)
    {
        #ifdef DEBUG_MODE
        if (static_cast<std::size_t>(g_register_table[index].id) != index)
        {
            throw std::logic_error(
                "Register ID and index table mismatch at index " + 
                std::to_string(index));
        }
        #endif
        return &g_register_table[index];
    }
    throw std::invalid_argument("Register ID out of table bounds: " + std::to_string(index));
}

const RegisterInfo *get_register_info(std::string_view key)
{
    for (std::size_t curr = 0; curr < g_register_table_size; ++curr)
    {
        if (key == g_register_table[curr].name)
            return &g_register_table[curr];
    }
    throw std::invalid_argument("Invalid register name " + std::string(key));
}

RegisterType get_register_type(RegisterID key)
{
    return get_register_info(key)->type;
}

RegisterType get_register_type(std::string_view key)
{
    return get_register_info(key)->type;
}

std::string_view
get_register_name(RegisterID id)
{
    return get_register_info(id)->name;
}

RegisterID
get_register_id(std::string_view name)
{
    return get_register_info(name)->id;
}

#elif defined(__x86_64__)

const RegisterInfo *get_register_info(RegisterID key)
{
    (void) key;
    throw std::logic_error("Register access not implemented for x86");
}

const RegisterInfo *get_register_info(std::string_view key)
{
    (void) key;
    throw std::logic_error("Register access not implemented for x86");
}

const RegisterType get_register_type(RegisterID key)
{
    return get_register_info(key)->type;
}

const RegisterType get_register_type(std::string_view key)
{
    return get_register_info(key)->type;
}

std::string_view
get_register_name(RegisterID id)
{
    return get_register_info(id)->name;
}

RegisterID
get_register_id(std::string_view name)
{
    return get_register_info(name)->id;
}

#endif

std::uint8_t *
Registers::register_offset(const RegisterInfo *info)
{
    std::uint8_t *offset = nullptr;

    if (info->type == RegisterType::RegGPR)
    {
        offset = static_cast<std::uint8_t *>(gpr_ptr());
        return (offset + info->offset);
    }
    else if (info->type == RegisterType::RegFPR)
    {
        offset = static_cast<std::uint8_t *>(fpr_ptr());
        return (offset + info->offset);
    }
    throw std::logic_error("Invalid register type in register_offset");
}

RegisterValue
parse_register_token(const RegisterInfo* info, std::string_view token)
{
    if (token.empty())
        throw std::invalid_argument("Empty register value");

    auto size_check_and_return =
    [&](uint64_t val, bool is_negative) -> RegisterValue
    {
        switch (info->size)
        {
            case 1:
                if (!is_negative && val > UINT8_MAX)
                    throw std::invalid_argument("Value too large for 8-bit register ");
                if (is_negative && static_cast<int64_t>(val) < INT8_MIN)
                    throw std::invalid_argument("Negative value too small for 8-bit register");
                return static_cast<uint8_t>(val);
            case 2:
                if (!is_negative && val > UINT16_MAX)
                    throw std::invalid_argument("Value too large for 16-bit register");
                if (is_negative && static_cast<int64_t>(val) < INT16_MIN)
                    throw std::invalid_argument("Negative value too small for 16-bit register");
                return static_cast<uint16_t>(val);
            case 4:
                if (!is_negative && val > UINT32_MAX)
                    throw std::invalid_argument("Value too large for 32-bit register");
                if (is_negative && static_cast<int64_t>(val) < INT32_MIN)
                    throw std::invalid_argument("Negative value too small for 32-bit register");
                return static_cast<uint32_t>(val);
            case 8:  return val;
            case 16: return static_cast<__uint128_t>(val);
            default: throw std::logic_error("Unsupported register size");
        }
    };

    auto size_check_float =
    [&](float val) -> RegisterValue
    {
        switch (info->size)
        {
            case 4:  return val;
            case 8:  return static_cast<double>(val);
            case 16: return static_cast<double>(val);
            default: throw std::invalid_argument("Float too large for register ");
        }
    };

    auto size_check_double = [&](double val) -> RegisterValue
    {
        if (info->size < 8)
            throw std::invalid_argument("Double too large for register ");
        return val;
    };

    if (token.size() > 2 && token[0] == '0' &&
       (token[1] == 'x' || token[1] == 'X'))
    {
        if (token.find('p') != std::string_view::npos ||
            token.find('P') != std::string_view::npos)
        {
            if (info->type != RegisterType::RegFPR)
                throw std::invalid_argument("Cannot assign float to non FPR register");

            bool is_float = (token.back() == 'f' || token.back() == 'F');
            std::string buf(is_float ? token.substr(0, token.size() - 1) : token);
            try
            {
                if (is_float) return size_check_float(std::stof(buf));
                else          return size_check_double(std::stod(buf));
            }
            catch (const std::out_of_range&)
            {
                throw std::invalid_argument("Hex float out of range");
            }
            catch (const std::invalid_argument&)
            { 
                throw std::invalid_argument("Invalid hex float");
            }

        }

        uint64_t val = 0;
        std::string_view hex = token.substr(2);
        auto [ptr, ec] = std::from_chars(hex.data(), hex.data() + hex.size(), val, 16);
        if (ec == std::errc::result_out_of_range)
            throw std::invalid_argument("Hex value out of range");
        if (ec != std::errc{} || ptr != hex.data() + hex.size())
            throw std::invalid_argument("Invalid hex value");

        return size_check_and_return(val, false);
    }

    if (token.find('.') != std::string_view::npos)
    {
        if (info->type != RegisterType::RegFPR)
            throw std::invalid_argument("Cannot assign float to non-FPR register");

        bool is_float = (token.back() == 'f' || token.back() == 'F');
        std::string buf(is_float ? token.substr(0, token.size() - 1) : token);
        try
        {
            if (is_float) return size_check_float(std::stof(buf));
            else          return size_check_double(std::stod(buf));
        }
        catch (const std::out_of_range&)
        {
            throw std::invalid_argument("Float out of range");
        }
        catch (const std::invalid_argument&)
        {
            throw std::invalid_argument("Invalid float");
        }
    }

    if (token[0] == '-')
    {
        int64_t val = 0;
        auto [ptr, ec] = std::from_chars(token.data(), token.data() + token.size(), val);
        if (ec == std::errc::result_out_of_range)
            throw std::invalid_argument("Signed value out of range");
        if (ec != std::errc{} || ptr != token.data() + token.size())
            throw std::invalid_argument("Invalid signed value");

        return size_check_and_return(static_cast<uint64_t>(val), true);
    }

    if (token[0] == '0' && token.size() > 1)
    {
        uint64_t val = 0;
        std::string_view oct = token.substr(1);
        auto [ptr, ec] = std::from_chars(oct.data(), oct.data() + oct.size(), val, 8);
        if (ec == std::errc::result_out_of_range)
            throw std::invalid_argument("Octal value out of range");
        if (ec != std::errc{} || ptr != oct.data() + oct.size())
            throw std::invalid_argument("Invalid octal value");

        return size_check_and_return(static_cast<uint64_t>(val), false);
    }

    uint64_t val = 0;
    auto [ptr, ec] = std::from_chars(token.data(), token.data() + token.size(), val);
    if (ec == std::errc::result_out_of_range)
        throw std::invalid_argument("Value out of range");
    if (ec != std::errc{} || ptr != token.data() + token.size())
        throw std::invalid_argument("Invalid value");
    return size_check_and_return(static_cast<uint64_t>(val), false);
}

RegisterValue Registers::read(const RegisterInfo* info)
{
    // Can't use bit_cast since I am using C++17
    // Using reinterpret_cast on unaligned memory
    // can cause a SIGBUS crash on AArch64

    std::uint8_t *src_addr = register_offset(info);
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

void Registers::write(const RegisterInfo* info, RegisterValue val)
{
    std::uint8_t *dest_addr = register_offset(info);
    auto func = [&](auto &&arg) -> void
    {
        if (sizeof(arg) > info->size)
            throw std::invalid_argument("Value too large for " + std::string(info->name));
        // AArch64 GPR Zero-extension rule (W -> X)
        if (info->type == RegisterType::RegGPR)
        {
            std::memset(dest_addr, 0, 8);
            std::memcpy(dest_addr, &arg, sizeof(arg));
            return;
        }

        std::memcpy(dest_addr, &arg, sizeof(arg));
    };
    std::visit(func, val);
}

void Registers::write(const RegisterInfo* info, std::string_view val)
{
    std::uint8_t *dest_addr = register_offset(info);
    RegisterValue parsed_val = parse_register_token(info, val);
    auto func = [&](auto &&arg) -> void
    {
        if (sizeof(arg) > info->size)
            throw std::invalid_argument("Value too large for " + std::string(info->name));
        // AArch64 GPR Zero-extension rule (W -> X)
        if (info->type == RegisterType::RegGPR)
        {
            std::memset(dest_addr, 0, 8);
            std::memcpy(dest_addr, &arg, sizeof(arg));
            return;
        }

        std::memcpy(dest_addr, &arg, sizeof(arg));
    };
    std::visit(func, parsed_val);
}

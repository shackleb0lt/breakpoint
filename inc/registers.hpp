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

#include <variant>
#include <cstddef>
#include <cstdint>
#include <string_view>

#if defined(__aarch64__) 
#include <asm/ptrace.h>
#elif defined(__x86_64__)
#include <sys/user.h>
#endif

using RegisterValue = std::variant
    <uint8_t, uint16_t, uint32_t, uint64_t, __uint128_t>;

#if defined(__aarch64__)

enum class RegisterID
{
    REG64_X0,  REG64_X1,  REG64_X2,  REG64_X3,  REG64_X4,  REG64_X5,  REG64_X6,  REG64_X7,
    REG64_X8,  REG64_X9,  REG64_X10, REG64_X11, REG64_X12, REG64_X13, REG64_X14, REG64_X15,
    REG64_X16, REG64_X17, REG64_X18, REG64_X19, REG64_X20, REG64_X21, REG64_X22, REG64_X23,
    REG64_X24, REG64_X25, REG64_X26, REG64_X27, REG64_X28, REG64_X29, REG64_X30,

    REG32_W0,  REG32_W1,  REG32_W2,  REG32_W3,  REG32_W4,  REG32_W5,  REG32_W6,  REG32_W7,
    REG32_W8,  REG32_W9,  REG32_W10, REG32_W11, REG32_W12, REG32_W13, REG32_W14, REG32_W15,
    REG32_W16, REG32_W17, REG32_W18, REG32_W19, REG32_W20, REG32_W21, REG32_W22, REG32_W23,
    REG32_W24, REG32_W25, REG32_W26, REG32_W27, REG32_W28, REG32_W29, REG32_W30,
     
    REG128_Q0,  REG128_Q1,  REG128_Q2,  REG128_Q3,  REG128_Q4,  REG128_Q5,  REG128_Q6,  REG128_Q7,
    REG128_Q8,  REG128_Q9,  REG128_Q10, REG128_Q11, REG128_Q12, REG128_Q13, REG128_Q14, REG128_Q15,
    REG128_Q16, REG128_Q17, REG128_Q18, REG128_Q19, REG128_Q20, REG128_Q21, REG128_Q22, REG128_Q23,
    REG128_Q24, REG128_Q25, REG128_Q26, REG128_Q27, REG128_Q28, REG128_Q29, REG128_Q30, REG128_Q31,
    
    REG64_D0,  REG64_D1,  REG64_D2,  REG64_D3,  REG64_D4,  REG64_D5,  REG64_D6,  REG64_D7,
    REG64_D8,  REG64_D9,  REG64_D10, REG64_D11, REG64_D12, REG64_D13, REG64_D14, REG64_D15,
    REG64_D16, REG64_D17, REG64_D18, REG64_D19, REG64_D20, REG64_D21, REG64_D22, REG64_D23,
    REG64_D24, REG64_D25, REG64_D26, REG64_D27, REG64_D28, REG64_D29, REG64_D30, REG64_D31,
    
    REG32_S0,  REG32_S1,  REG32_S2,  REG32_S3,  REG32_S4,  REG32_S5,  REG32_S6,  REG32_S7,
    REG32_S8,  REG32_S9,  REG32_S10, REG32_S11, REG32_S12, REG32_S13, REG32_S14, REG32_S15,
    REG32_S16, REG32_S17, REG32_S18, REG32_S19, REG32_S20, REG32_S21, REG32_S22, REG32_S23,
    REG32_S24, REG32_S25, REG32_S26, REG32_S27, REG32_S28, REG32_S29, REG32_S30, REG32_S31,
    
    REG16_H0,  REG16_H1,  REG16_H2,  REG16_H3,  REG16_H4,  REG16_H5,  REG16_H6,  REG16_H7,
    REG16_H8,  REG16_H9,  REG16_H10, REG16_H11, REG16_H12, REG16_H13, REG16_H14, REG16_H15,
    REG16_H16, REG16_H17, REG16_H18, REG16_H19, REG16_H20, REG16_H21, REG16_H22, REG16_H23,
    REG16_H24, REG16_H25, REG16_H26, REG16_H27, REG16_H28, REG16_H29, REG16_H30, REG16_H31,
    
    REG8_B0,  REG8_B1,  REG8_B2,  REG8_B3,  REG8_B4,  REG8_B5,  REG8_B6,  REG8_B7,
    REG8_B8,  REG8_B9,  REG8_B10, REG8_B11, REG8_B12, REG8_B13, REG8_B14, REG8_B15,
    REG8_B16, REG8_B17, REG8_B18, REG8_B19, REG8_B20, REG8_B21, REG8_B22, REG8_B23,
    REG8_B24, REG8_B25, REG8_B26, REG8_B27, REG8_B28, REG8_B29, REG8_B30, REG8_B31,

    REG64_PC, REG64_SP, REG64_PSTATE, REG32_FPSR, REG32_FPCR
};

#elif defined(__x86_64__)

enum class RegisterID
{
    REG64_RAX, REG64_RBX, REG64_RCX, REG64_RDX, REG64_RSI, REG64_RDI, REG64_RBP, REG64_RSP,
    REG64_R8,  REG64_R9,  REG64_R10, REG64_R11, REG64_R12, REG64_R13, REG64_R14, REG64_R15,
    REG64_RIP,
    REG64_EFLAGS,
    REG64_CS,
    REG64_FS,
    REG64_GS,
    REG64_SS,
    REG64_DS,
    REG64_ES,
    REG64_ORIG_RAX,

    REG32_EAX, REG32_EBX, REG32_ECX,  REG32_EDX,  REG32_ESI,  REG32_EDI,  REG32_EBP,  REG32_ESP,
    REG32_R8D, REG32_R9D, REG32_R10D, REG32_R11D, REG32_R12D, REG32_R13D, REG32_R14D, REG32_R15D,

    REG16_AX,  REG16_BX,  REG16_CX,   REG16_DX,   REG16_SI,   REG16_DI,   REG16_BP,   REG16_SP,
    REG16_R8W, REG16_R9W, REG16_R10W, REG16_R11W, REG16_R12W, REG16_R13W, REG16_R14W, REG16_R15W,

    REG8H_AH,
    REG8H_BH,
    REG8H_CH,
    REG8H_DH,

    REG8L_AL,  REG8L_BL,  REG8L_CL,   REG8L_DL,   REG8L_SIL,  REG8L_DIL,  REG8L_BPL,  REG8L_SPL,
    REG8L_R8B, REG8L_R9B, REG8L_R10B, REG8L_R11B, REG8L_R12B, REG8L_R13B, REG8L_R14B, REG8L_R15B,

    REG64_DR0, REG64_DR1, REG64_DR2, REG64_DR3, REG64_DR4, REG64_DR5, REG64_DR6, REG64_DR7,

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
};

#endif

class Registers
{
    private:
#if defined(__aarch64__) 
        struct user_pt_regs gpr_{};
        struct user_fpsimd_state fpr_{};
#elif defined(__x86_64__)
        struct user_regs_struct gpr_{};
        struct user_fpregs_struct fpr_{};
#endif

        friend class Process;
        Registers() = default;

    public:
        Registers(const Registers&) = delete;
        Registers& operator=(const Registers&) = delete;

        void* gpr_ptr() { return &gpr_;}
        void* fpr_ptr() { return &fpr_;}
        std::size_t gpr_size() { return sizeof(gpr_);}
        std::size_t fpr_size() { return sizeof(fpr_);}

        RegisterValue read(std::string_view reg_name);
        RegisterValue read(RegisterID reg_id);

        void write(std::string_view reg_name, RegisterValue val);
        void write(std::string_view reg_name, std::string_view val_str);
};

#endif
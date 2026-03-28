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

#include "process.hpp"
#include "disassembler.hpp"
#include <capstone/capstone.h>

std::vector<instruction>
Disassembler::disassemble(std::size_t count, std::optional<virt_addr> address)
{
    csh handle;
    cs_insn *insn;
    size_t code_size = count * 4;
    std::vector<instruction> ret;

    if (!address)
    {
        address.emplace(process_->get_pc());
    }

    auto code = process_->read_memory_without_traps(*address, code_size);
    code_size = code.size();

    cs_err err = cs_open(CS_ARCH_ARM64, CS_MODE_ARM, &handle);
    if (err != CS_ERR_OK)
    {
        // Error::send("Failed to intialize capstone");
        Error::send("Failed to intialize capstone: " + std::string(cs_strerror(err)));
    }

    cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);

    const uint8_t *code_ptr = reinterpret_cast<const uint8_t *>(code.data());
    count = cs_disasm(handle, code_ptr, code_size, *address, 0, &insn);

    if (count == 0)
    {
        cs_close(&handle);
        Error::send("Failed to disassemble: " + std::string(cs_strerror(cs_errno(handle))));
    }

    ret.reserve(count);
    for (size_t i = 0; i < count; i++)
    {
        instruction ins;
        ins.addr = insn[i].address;
        ins.text = insn[i].mnemonic;
        ins.text += " ";
        ins.text += insn[i].op_str;

        ret.push_back(ins);
    }

    cs_free(insn, count);
    cs_close(&handle);

    return ret;
}

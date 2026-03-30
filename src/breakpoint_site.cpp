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

#include "breakpoint_site.hpp"
#include "process.hpp"
#include "error.hpp"

#include <sys/ptrace.h>

namespace 
{
    BreakpointSite::id_type get_next_id()
    {
        static BreakpointSite::id_type id = 0;
        return ++id;
    }
}

BreakpointSite::BreakpointSite(Process &proc,
    virt_addr address, bool is_hw, bool is_int)
{
    address_ = address;
    process_ = &proc;
    is_enabled_ = false;
    is_hardware_ = is_hw;
    is_internal_ = is_int;
    saved_data_ = 0;
    id_ = is_internal_? -1 : get_next_id();
}

void BreakpointSite::enable()
{
    if (is_enabled_)
        return;

    if (is_hardware_)
    {
        hw_register_ind_ = process_->set_hw_breakpoint(address_);
        is_enabled_ = true;
        return;
    }

    errno = 0;
    std::uint64_t data = ptrace(PTRACE_PEEKDATA, process_->get_pid(), address_, nullptr);
    if (errno != 0)
    {
        Error::send_errno("Failed to enable breakpoint site");
    }
    
    saved_data_ = data;
    data = (data & 0xFFFFFFFF00000000LL) | 0xD4200000LL;
    #if 0
    // Turns out below calculation was needed, linux always return the instruction
    // in lower 32 bits of the data irrespective of whether address is 8 bye aligned or not
    // if (address_ % 8 == 0)
    // {
    //     data = (data & 0xFFFFFFFF00000000LL) | 0xD4200000LL;
    // }
    // else 
    // {
    //     data = (data & 0x00000000FFFFFFFFLL) | (0xD4200000LL << 32);
    // }
    #endif

    if (ptrace(PTRACE_POKEDATA, process_->get_pid(), address_, data) < 0)
    {
        Error::send_errno("Failed to enable breakpoint site");
    }

    is_enabled_ = true;
}

void BreakpointSite::disable()
{
    if (!is_enabled_)
        return;
    
    if (is_hardware_)
    {
        process_->clear_hw_breakpoint(hw_register_ind_);
        hw_register_ind_ = -1;
        is_enabled_ = false;
        return;
    }

    errno = 0;
    std::uint64_t data = ptrace(PTRACE_PEEKDATA, process_->get_pid(), address_, nullptr);
    if (errno != 0)
    {
        Error::send_errno("Failed to disable breakpoint site");
    }
    
    data = saved_data_;
    if (ptrace(PTRACE_POKEDATA, process_->get_pid(), address_, data) < 0)
    {
        Error::send_errno("Failed to disable breakpoint site");
    }

    is_enabled_ = false;
}
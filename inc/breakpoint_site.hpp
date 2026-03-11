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

#ifndef BKPT_LIB_BREAKPOINT_SITE_H
#define BKPT_LIB_BREAKPOINT_SITE_H

#include <cstdint>
#include <cstddef>

using virt_addr = std::uint64_t;

class Process;

class BreakpointSite
{
public:
    BreakpointSite() = delete;
    BreakpointSite(const BreakpointSite &) = delete;
    BreakpointSite &operator=(const BreakpointSite &) = delete;

    using id_type = std::uint32_t;
    id_type id() const { return id_; }

    void enable();
    void disable();

    bool is_enabled() const { return is_enabled_; }
    virt_addr address() const { return address_; }

    bool at_address(virt_addr addr) const
    {
        return address_ == addr;
    }

    bool in_range(virt_addr low, virt_addr high) const
    {
        return (low <= address_) && (high > address_);
    }

private:
    friend Process;
    BreakpointSite(Process &proc, virt_addr address);

    id_type id_;
    bool is_enabled_;
    virt_addr address_;
    std::uint64_t saved_data_;
    Process* process_;
};

#endif
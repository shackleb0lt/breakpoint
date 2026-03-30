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

#ifndef BKPT_LIB_PROCESS_H
#define BKPT_LIB_PROCESS_H

#include <memory>
#include <optional>
#include <string_view>
#include <sys/types.h>

#include "types.hpp"
#include "registers.hpp"
#include "stoppoint_collection.hpp"
#include "breakpoint_site.hpp"

enum class ProcessState : uint8_t
{
    Init = 0,
    Stopped,
    Running,
    Exited,
    Terminated,
};

class Process
{
public:
    Process() = delete;
    ~Process();

    Process(const Process &) = delete;
    Process &operator=(const Process &) = delete;

    static std::unique_ptr<Process>
    launch(std::vector<std::string_view> &exec_args,
        std::optional<int*> comm = std::nullopt);

    static std::unique_ptr<Process>
    attach(pid_t pid);

    std::uint8_t wait();
    void resume();
    std::uint8_t step_instruction();

    pid_t get_pid() { return pid_; }
    ProcessState get_state() { return state_; }

    virt_addr get_pc();
    void set_pc(virt_addr address);

    Registers &registers() { return *reg_state_; }
    const Registers &registers() const { return *reg_state_; }

    BreakpointSite& create_breakpoint_site(
        virt_addr address, bool hw = false, bool intnl = false);

    int set_hw_breakpoint(virt_addr addr);
    int set_hw_watchpoint(virt_addr addr);
    void clear_hw_breakpoint(int index);
    void clear_hw_watchpoint(int index);

    StoppointCollection<BreakpointSite>&
    breakpoint_sites() { return breakpoint_sites_; }
    const StoppointCollection<BreakpointSite>&
    breakpoint_sites() const { return breakpoint_sites_; }

    std::vector<std::uint8_t>
    read_memory(virt_addr address, std::size_t size) const;
    std::vector<std::uint8_t>
    read_memory_without_traps(virt_addr address, std::size_t size) const;
    void write_memory(virt_addr address, Span<const std::uint8_t> data);

#ifdef DEBUG_MODE
    std::vector<std::uint32_t>
    get_instructions(virt_addr addr, std::size_t count);
#endif

private:
    Process(pid_t pid, bool kill_on_end) : 
        pid_(pid), kill_on_end_(kill_on_end), reg_state_(new Registers(*this)){}
    void get_registers();
    void set_registers();

    pid_t pid_ = 0;
    bool kill_on_end_ = true;
    ProcessState state_ = ProcessState::Init;
    std::unique_ptr<Registers> reg_state_;
    StoppointCollection<BreakpointSite> breakpoint_sites_;
};

#endif

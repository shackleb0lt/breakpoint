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

#include <cstdlib>
#include <iostream>
#include <string>
#include <charconv>
#include <cstring>
#include <type_traits>

#include <fmt/core.h>
#include <fmt/format.h>

#include "linenoise.h"
#include "commands.hpp"
#include "error.hpp"
#include "process.hpp"
#include "disassembler.hpp"

#define COMMANDS_HISTORY "/tmp/breakpoint.txt"

using ProcessPtr = std::unique_ptr<Process>;

void print_usage(std::string_view exe_name)
{
    std::cout << "Usage: " << exe_name << " -p <pid>\n"
              << "Usage: " << exe_name << "<executable-file> [args]" << std::endl;
}

void print_stop_reason(ProcessPtr &proc, std::uint8_t ret)
{
    switch (proc->get_state())
    {
        case ProcessState::Exited:
            fmt::println("Process exited with status {}", static_cast<int>(ret));
            break;
        case ProcessState::Terminated:
            fmt::println("Process terminated with signal {}", sigabbrev_np(ret));
            break;
        case ProcessState::Stopped:
            fmt::println("Process stopped with signal {} at {:#016x}",
                sigabbrev_np(ret), proc->get_pc());
            break;
        default:
            break;
    }
}

bool sv_is_hex(std::string_view sv)
{
    return (sv.size() >= 2) && (sv[0] == '0') && (sv[1] == 'x' || sv[1] == 'X');
}

std::uint64_t to_positive_integral(std::string_view token)
{
    uint64_t val = 0;

    if (sv_is_hex(token))
    {
        std::string_view hex = token.substr(2);
        auto [ptr, ec] = std::from_chars(hex.data(), hex.data() + hex.size(), val, 16);
        if (ec == std::errc::result_out_of_range)
            throw std::invalid_argument("Hex value out of range");
        if (ec != std::errc{} || ptr != hex.data() + hex.size())
            throw std::invalid_argument("Invalid hex value");

        return val;
    }

    if (token[0] == '0' && token.size() > 1)
    {
        std::string_view oct = token.substr(1);
        auto [ptr, ec] = std::from_chars(oct.data(), oct.data() + oct.size(), val, 8);
        if (ec == std::errc::result_out_of_range)
            throw std::invalid_argument("Octal value out of range");
        if (ec != std::errc{} || ptr != oct.data() + oct.size())
            throw std::invalid_argument("Invalid octal value");

        return val;
    }

    auto [ptr, ec] = std::from_chars(token.data(), token.data() + token.size(), val);
    if (ec == std::errc::result_out_of_range)
        throw std::invalid_argument("Value out of range");
    if (ec != std::errc{} || ptr != token.data() + token.size())
        throw std::invalid_argument("Invalid value");
    return val;

}

std::vector<std::uint8_t>
parse_vector(std::string_view token)
{
    auto invalid = []{ throw std::invalid_argument("Invalid format"); };
    std::vector<std::uint8_t> bytes;
    const char* c = token.data();
    const char* end = token.data() + token.size();

    if (c >= end || *c != '[') invalid();
    c++;

    while (c < end && *c != ']')
    {
        // skip whitespace and commas
        while (c < end && (*c == ' ' || *c == ',')) c++;
        if (c >= end || *c == ']') break;

        // expect 0x prefix
        if (c + 2 >= end || *c != '0' || (*(c+1) != 'x' && *(c+1) != 'X'))
            invalid();
        c += 2;

        uint8_t val = 0;
        auto [ptr, ec] = std::from_chars(c, end, val, 16);
        if (ec != std::errc{}) invalid();
        c = ptr;

        bytes.push_back(val);
    }

    if (c >= end || *c != ']') invalid();
    return bytes;
}

void display_register(const RegisterID id, const RegisterValue& val)
{
    fmt::print("{:<6}: ", get_register_name(id));

    std::visit([&](auto&& arg)
    {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, __uint128_t>)
        {
            uint64_t high   = static_cast<uint64_t>(arg >> 64);
            uint64_t low    = static_cast<uint64_t>(arg);
            uint32_t low_32 = static_cast<uint32_t>(low);

            double as_double;  float as_float;
            std::memcpy(&as_double, &low,    sizeof(double));
            std::memcpy(&as_float,  &low_32, sizeof(float));

            fmt::print("0x{:016x}{:016x} | dbl:{} | flt:{}",
                high, low, as_double, as_float);
            return;
        }

        if constexpr (std::is_same_v<T, uint64_t> || std::is_same_v<T, uint32_t>)
        {
            if (get_register_type(id) == RegisterType::RegFPR)
            {
                if constexpr (std::is_same_v<T, uint64_t>)
                {
                    double dbl;
                    std::memcpy(&dbl, &arg, sizeof(double));
                    fmt::print("0x{:016x} (dbl:{})", arg, dbl);
                }
                else
                {
                    float flt;
                    std::memcpy(&flt, &arg, sizeof(float));
                    fmt::print("0x{:016x} (flt:{})", arg, flt);
                }
                return;
            }
        }

        if constexpr (std::is_integral_v<T>)
        {
            using SignedT = std::make_signed_t<T>;
            uint64_t uval = static_cast<uint64_t>(arg);
            int64_t  sval = static_cast<int64_t>(static_cast<SignedT>(arg));
            fmt::print("0x{:016x} (u:{} s:{})", uval, uval, sval);
        }

    }, val);
}

void display_memory(std::vector<std::uint8_t> &data, virt_addr addr)
{
    for (std::size_t i = 0; i < data.size(); i += 16)
    {
        fmt::print("{:#016x}: ", addr + i);
        auto end = std::min(i + 16, data.size());
        for (std::size_t j = i; j < end; ++j)
            fmt::print("{:02x} ", data[j]);

        fmt::print("\n");
    }
}

void display_disassembly(ProcessPtr &proc, virt_addr addr = 0, std::size_t count = 5)
{
    Disassembler dis(*proc);
    if (addr == 0)
        addr = proc->get_pc();

    auto insn = dis.disassemble(count, addr);
    for (auto &ins : insn)
    {
        fmt::print("{:#018x}: {}\n", ins.addr, ins.text);
    }
}

void display_breakpoints(ProcessPtr &proc)
{
    if (proc->breakpoint_sites().empty())
    {
        fmt::println("No breakpoints set");
        return;
    }

    fmt::println("Current breakpoints:");
    auto func = [](BreakpointSite& site)
    {
        if (site.is_internal())
            return;

        fmt::print("{}: address = {:#x}, {}\n",
            site.id(), site.address(),
            site.is_enabled() ? "enabled" : "disabled");
    };

    proc->breakpoint_sites().for_each(func);
}

bool handle_command(std::string_view line, ProcessPtr &proc)
{
    auto [action, tokens] = process_line(line);

    if (action == Action::Quit)
    {
        return false;
    }
    else if (action == Action::None)
    {
        return true;
    }
    else if (action == Action::Invalid)
    {
        std::cout << "Invalid Command" << std::endl;
        return true;
    }
    else if (action == Action::Incomplete)
    {
        std::cout << "Incomplete Command" << std::endl;
        return true;
    }
    else if (action == Action::Ambiguous)
    {
        std::cout << "Ambiguous Command" << std::endl;
        return true;
    }

    try
    {
        if (action == Action::Continue)
        {
            proc->resume();
            std::uint8_t ret = proc->wait();
            print_stop_reason(proc, ret);
            if (proc->get_state() == ProcessState::Stopped)
                display_disassembly(proc);
        }
        else if(action == Action::StepInst)
        {
            std::uint8_t ret = proc->step_instruction();
            print_stop_reason(proc, ret);
            if (proc->get_state() == ProcessState::Stopped)
                display_disassembly(proc);
        }
        else if (action == Action::ReadReg)
        {
            RegisterValue val = proc->registers().read<RegisterValue>(tokens[2]);
            display_register(get_register_id(tokens[2]), val);
        }
        else if (action == Action::ReadRegGPR)
        {
            std::size_t curr = 0;
            std::size_t end = static_cast<std::size_t>(RegisterID::REG32_W30);
            for (; curr <= end; curr++)
            {
                RegisterID id = static_cast<RegisterID>(curr);
                RegisterValue val = proc->registers().read<RegisterValue>(id);
                display_register(id, val);
            }
        }
        else if (action == Action::ReadRegAll)
        {
            std::size_t curr = 0;
            std::size_t end = static_cast<std::size_t>(RegisterID::REG32_FPCR);
            for (; curr <= end; curr++)
            {
                RegisterID id = static_cast<RegisterID>(curr);
                RegisterValue val = proc->registers().read<RegisterValue>(id);
                display_register(id, val);
            }
        }
        else if (action == Action::WriteReg)
        {
            proc->registers().write(tokens[2], tokens[3]);
        }
        else if (action == Action::Disassmbl)
        {
            display_disassembly(proc);
        }
        else if (action == Action::Disassmbl1)
        {
            if (sv_is_hex(tokens[1]))
                display_disassembly(proc, to_positive_integral(tokens[1]));
            else
                display_disassembly(proc, proc->get_pc(), to_positive_integral(tokens[1]));
        }
        else if (action == Action::Disassmbl2)
        {
            if (sv_is_hex(tokens[1]))
                display_disassembly(proc, to_positive_integral(tokens[1]), to_positive_integral(tokens[2]));
            else if (sv_is_hex(tokens[2]))
                display_disassembly(proc, to_positive_integral(tokens[2]), to_positive_integral(tokens[1]));
            else
                throw std::invalid_argument("Address must be in hex format");
        }
        else if (action == Action::MemReadDef)
        {
            virt_addr address = to_positive_integral(tokens[2]);
            auto data = proc->read_memory(address, 32);
            display_memory(data, address);
        }
        else if (action == Action::MemReadCnt)
        {
            virt_addr address = to_positive_integral(tokens[2]);
            uint64_t count = to_positive_integral(tokens[3]);
            auto data = proc->read_memory(address, count);
            display_memory(data, address);
        }
        else if (action == Action::MemWrite)
        {
            virt_addr address = to_positive_integral(tokens[2]);
            auto data = parse_vector(tokens[3]);
            proc->write_memory(address, {data.data(), data.size()});
        }
        else if (action == Action::BPSiteList)
        {
            display_breakpoints(proc);
        }
        else if (action == Action::BPSiteSet)
        {
            virt_addr address = to_positive_integral(tokens[2]);
            proc->create_breakpoint_site(address).enable();
        }
        else if (action == Action::BPSiteSetHW)
        {
            virt_addr address = to_positive_integral(tokens[2]);
            proc->create_breakpoint_site(address, true).enable();
        }
        else if (action == Action::BPSiteEn)
        {
            auto id = static_cast<BreakpointSite::id_type>(to_positive_integral(tokens[2]));
            proc->breakpoint_sites().get_by_id(id).enable();
        }
        else if (action == Action::BPSiteDis)
        {
            auto id = static_cast<BreakpointSite::id_type>(to_positive_integral(tokens[2]));
            proc->breakpoint_sites().get_by_id(id).disable();
        }
        else if (action == Action::BPSiteDel)
        {
            auto id = static_cast<BreakpointSite::id_type>(to_positive_integral(tokens[2]));
            proc->breakpoint_sites().remove_by_id(id);
        }
    }
    catch(const std::invalid_argument& err)
    {
        std::cout << "Error: "<< err.what() << std::endl;
        return true;
    }
    catch(const std::logic_error &err)
    {
        std::cout << "Internal Error: "<< err.what() << std::endl;
        return false;
    }
    catch (const Error &err)
    {
        std::cout << "Error occured for " << tokens[0]
                  << ": " << err.what() << std::endl;
        return false;
    }

    return true;
}

void cli_repl(ProcessPtr &proc)
{
    char *raw_line = NULL;

    linenoiseHistorySetMaxLen(200);
    linenoiseHistoryLoad(COMMANDS_HISTORY);

    std::cout << "Welcome to breakpoint!" << std::endl;
    std::cout << "Attached process ID is: " << proc->get_pid() << std::endl;

    while ((raw_line = linenoise("bkpt> ")) != NULL)
    {
        // Do nothing if line is empty
        // TODO: Can treat this as shortcut to run previous command
        if (raw_line[0] == '\0')
        {
            free(raw_line);
            continue;
        }

        if (handle_command(raw_line, proc) == false)
        {
            free(raw_line);
            break;
        }

        linenoiseHistoryAdd(raw_line);
        free(raw_line);
    }

    linenoiseHistorySave(COMMANDS_HISTORY);
}

int main(int argc, char *argv[])
{
    int debug_pid = 0;
    ProcessPtr proc;

    std::vector<std::string_view> args(argv, argv + argc);
    const std::string_view program_name = args[0];

    if (argc == 1)
    {
        print_usage(program_name);
        return 1;
    }

    try
    {
        if (args[1] == "-h" || args[1] == "--help")
        {
            print_usage(program_name);
            return 0;
        }
        else if (args[1] == "-p")
        {
            if (argc < 3)
                throw std::runtime_error("Missing PID after -p");

            try
            {
                debug_pid = std::stoi(std::string(args[2]));
            }
            catch (...)
            {
                throw std::runtime_error("Unable to parse process id [" + std::string(args[2]) + "]");
            }

            if (debug_pid <= 0)
                throw std::runtime_error("PID must be positive");

            proc = Process::attach(debug_pid);
        }
        else
        {

            std::vector<std::string_view> exec_args(args.begin() + 1, args.end());
            proc = Process::launch(exec_args);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        print_usage(program_name);
        return 1;
    }

    cli_repl(proc);

    return 0;
}

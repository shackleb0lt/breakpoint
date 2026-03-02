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

#include <cstring>
#include <type_traits>

#include <fmt/core.h>
#include <fmt/format.h>

#include "linenoise.h"
#include "commands.hpp"
#include "error.hpp"
#include "process.hpp"

#define COMMANDS_HISTORY "/tmp/breakpoint.txt"

using ProcessPtr = std::unique_ptr<Process>;

void print_usage(std::string_view exe_name)
{
    std::cout << "Usage: " << exe_name << " -p <pid>\n"
              << "Usage: " << exe_name << "<executable-file> [args]" << std::endl;
}

void print_stop_reason(ProcessState state, std::uint8_t ret)
{
    switch (state)
    {
        case ProcessState::Exited:
            std::cout << "Process exited with status "
                      << static_cast<int>(ret) << std::endl;
            break;
        case ProcessState::Terminated:
            std::cout << "Process terminated with signal "
                      << sigabbrev_np(ret) << std::endl;
            break;
        case ProcessState::Stopped:
            std::cout << "Process stopped with signal "
                      << sigabbrev_np(ret) << std::endl;
            break;
        default:
            break;
    }
}

void display_register(std::string_view name, const RegisterValue& val)
{
    std::string output = fmt::format("{:<6}: ", name);

    std::visit([&output](auto&& arg)
    {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, float>)
        {
            uint32_t bits;
            std::memcpy(&bits, &arg, sizeof(bits));
            output += fmt::format("0x{:<16x} (flt:{})", bits, arg);
        }
        else if constexpr(std::is_same_v<T, double>)
        {
            uint64_t bits;
            std::memcpy(&bits, &arg, sizeof(bits));
            output += fmt::format("0x{:<16x} (dbl:{})", bits, arg);
        }
        else if constexpr (std::is_same_v<T, __uint128_t>)
        {
            uint64_t high   = static_cast<uint64_t>(arg >> 64);
            uint64_t low    = static_cast<uint64_t>(arg);
            uint32_t low_32 = static_cast<uint32_t>(low);

            double as_double;
            std::memcpy(&as_double, &low, sizeof(double));
            float as_float;
            std::memcpy(&as_float, &low_32, sizeof(float));

            output += fmt::format("0x{:016x}{:016x} | dbl:{} | flt:{}", 
                                  high, low, as_double, as_float);
        } 
        else
        {
            using SignedT = std::make_signed_t<T>;
            uint64_t uval = static_cast<uint64_t>(arg);
            int64_t  sval = static_cast<int64_t>(static_cast<SignedT>(arg));
            output += fmt::format("0x{:<16x} (u:{} s:{})", uval, uval, sval);
        }
    }, val);

    fmt::println("{}", output);
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
            std::uint8_t ret =  proc->wait();
            print_stop_reason(proc->get_state(), ret);
        }
        else if (action == Action::ReadReg)
        {
            RegisterValue val = proc->read_register(tokens[2]);
            display_register(tokens[2], val);
        }
        else if (action == Action::ReadRegGPR)
        {
            ;
        }
        else if (action == Action::ReadRegAll)
        {
            ;
        }
        else if (action == Action::WriteReg)
        {
            proc->write_register(tokens[2], tokens[3]);
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

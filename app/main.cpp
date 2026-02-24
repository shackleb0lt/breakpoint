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
#include <memory>
#include <stdexcept>
#include <string>

#include "linenoise.h"
#include "commands.hpp"
#include "error.hpp"
#include "process.hpp"

void print_usage(std::string_view exe_name)
{
    std::cout << "Usage: " << exe_name << " -p <pid>\n"
              << "Usage: " << exe_name << "<executable-file> [args]" << std::endl;
}

bool handle_command(std::string_view line, std::unique_ptr<Process> &proc)
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
            proc->wait();
        }
    }
    catch (const Error &err)
    {
        std::cout << "Error occured for " << tokens[0]
                  << ": " << err.what() << std::endl;
        return false;
    }

    return true;
}

void cli_repl(std::unique_ptr<Process> &proc)
{
    char *raw_line = NULL;
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
}

int main(int argc, char *argv[])
{
    int debug_pid = 0;

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

        std::unique_ptr<Process> proc;

        if (args[1] == "-p")
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

        cli_repl(proc);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        print_usage(program_name);
        return 1;
    }

    return 0;
}
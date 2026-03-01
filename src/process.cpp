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
#include "pipe.hpp"
#include "error.hpp"

#include <csignal>
#include <string>
#include <charconv>
#include <algorithm>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>

#include <sys/user.h>
#include <sys/uio.h>      // Required for iovec
#include <elf.h>          // Required for NT_PRSTATUS

void exit_with_perror(Pipe &pipe, std::string_view prefix)
{
    std::string msg{prefix};
    msg += ": ";
    msg += std::strerror(errno);

    try
    {
        pipe.write(msg);
    }
    catch (...)
    {
        // Double fault
        // Can't do anything except escape silently
    }

    ::exit(-1);
}

Process::~Process()
{
    int status = 0;
    if (pid_ <= 0)
        return;

    if (state_ == ProcessState::Exited ||
        state_ == ProcessState::Terminated)
        return;

    if (state_ == ProcessState::Running)
    {
        kill(pid_, SIGSTOP);
        waitpid(pid_, &status, 0);
        state_ = ProcessState::Stopped;
    }

    ptrace(PTRACE_DETACH, pid_, nullptr, nullptr);
    kill(pid_, SIGCONT);
    state_ = ProcessState::Running;

    if (kill_on_end_ == true)
    {
        kill(pid_, SIGKILL);
        waitpid(pid_, &status, 0);
        state_ = ProcessState::Terminated;
    }
}

std::uint8_t Process::wait()
{
    int status = 0;
    std::uint8_t info = 0;
    if (waitpid(pid_, &status, 0) < 0)
    {
        Error::send_errno("waitpid() failed");
    }

    if (WIFEXITED(status))
    {
        state_ = ProcessState::Exited;
        info = WEXITSTATUS(status);
    }
    else if (WIFSIGNALED(status))
    {
        state_ = ProcessState::Terminated;
        info = WTERMSIG(status);
    }
    else if (WIFSTOPPED(status))
    {
        state_ = ProcessState::Stopped;
        info = WSTOPSIG(status);
    }

    if (state_ == ProcessState::Stopped)
    {
        get_registers();
    }

    return info;
}

void Process::resume()
{
    set_registers();

    if (ptrace(PTRACE_CONT, pid_, nullptr, nullptr) < 0)
    {
        Error::send_errno("ptrace(PTRACE_CONT) failed");
    }

    state_ = ProcessState::Running;
}

std::unique_ptr<Process>
Process::launch(std::vector<std::string_view> &exec_args)
{
    if (exec_args.empty())
    {
        Error::send("No executable provided");
    }

    Pipe channel(true);
    pid_t debug_pid = ::fork();

    if (debug_pid < 0)
    {
        Error::send_errno("fork() failed");
    }

    if (debug_pid == 0)
    {
        channel.close_read();
        if (::ptrace(PTRACE_TRACEME, 0, nullptr, nullptr) < 0)
        {
            exit_with_perror(channel, "Tracing Failed");
        }

        std::vector<std::string> args_copy;
        args_copy.reserve(exec_args.size());
        for (const auto &arg : exec_args)
            args_copy.emplace_back(arg);

        std::vector<char *> argv;
        argv.reserve(args_copy.size() +1);
        for (auto &arg : args_copy)
            argv.push_back(arg.data());

        argv.push_back(nullptr);

        std::string path = std::string(exec_args[0]);
        if (path.find('/') == std::string::npos)
        {
            std::string local_path = "./" + path;
            if (::access(local_path.c_str(), X_OK) == 0)
            {
                path = local_path;
            }
        }

        execvp(path.c_str(), argv.data());
        exit_with_perror(channel, "execvp() failed for " + path);
    }

    channel.close_write();

    std::string error_msg;
    channel.read(error_msg);
    channel.close_read();

    if (error_msg.empty() == false)
    {
        waitpid(debug_pid, nullptr, 0);
        Error::send("Launch error: " + error_msg);
    }

    std::unique_ptr<Process> proc(new Process(debug_pid, true));
    proc->wait();

    // Launched process should ideally stop execution
    // Due to a SIGTRAP recived just after execvp

    return proc;
}

std::unique_ptr<Process>
Process::attach(pid_t pid)
{
    if (pid <= 0)
    {
        Error::send("Invalid PID");
    }

    if (ptrace(PTRACE_ATTACH, pid, nullptr, nullptr) < 0)
    {
        Error::send_errno("Attach failed");
    }

    std::unique_ptr<Process> proc(new Process(pid, false));
    proc->wait();

    // Attached process should ideally stop execution
    // Due to a SIGSTOP received as part of PTRACE_ATTACH

    return proc;
}

void Process::get_registers()
{
    struct iovec iov;
    iov.iov_base = reg_state_.gpr_ptr();
    iov.iov_len = reg_state_.gpr_size();
    if (ptrace(PTRACE_GETREGSET, pid_, NT_PRSTATUS, &iov) < 0)
    {
        Error::send_errno("Failed to read general purpose registers");
    }

    iov.iov_base = reg_state_.fpr_ptr();
    iov.iov_len = reg_state_.fpr_size();
    if (ptrace(PTRACE_GETREGSET, pid_, NT_FPREGSET, &iov) < 0)
    {
        Error::send_errno("Failed to read vector registers");
    }
}

void Process::set_registers()
{
    struct iovec iov;
    iov.iov_base = reg_state_.gpr_ptr();
    iov.iov_len = reg_state_.gpr_size();
    if (ptrace(PTRACE_SETREGSET, pid_, NT_PRSTATUS, &iov) < 0)
    {
        Error::send_errno("Failed to read general purpose registers");
    }

    iov.iov_base = reg_state_.fpr_ptr();
    iov.iov_len = reg_state_.fpr_size();
    if (ptrace(PTRACE_SETREGSET, pid_, NT_FPREGSET, &iov) < 0)
    {
        Error::send_errno("Failed to read vector registers");
    }
}

RegisterValue Process::read_register(std::string_view reg_name)
{
    return reg_state_.read(reg_name);
}

RegisterValue Process::read_register(RegisterID reg_id)
{
    return reg_state_.read(reg_id);
}

void Process::write_register(std::string_view reg_name, std::string_view val_str)
{
    reg_state_.write(reg_name, val_str);
}

void Process::write_register(std::string_view reg_name, RegisterValue val)
{
    reg_state_.write(reg_name, val);
}
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
#include "registers.hpp"
#include "pipe.hpp"
#include "error.hpp"

#include <cstring>
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
    iov.iov_base = &(data_.regs);
    iov.iov_len = sizeof(data_.regs);

    if (ptrace(PTRACE_GETREGSET, pid_, NT_PRSTATUS, &iov) < 0)
    {
        Error::send_errno("Failed to read general purpose registers");
    }

    iov.iov_base = &(data_.fp_regs);
    iov.iov_len = sizeof(data_.fp_regs);

    if (ptrace(PTRACE_GETREGSET, pid_, NT_FPREGSET, &iov) < 0)
    {
        Error::send_errno("Failed to read vector registers");
    }
}

void Process::set_registers()
{
    struct iovec iov;
    iov.iov_base = &(data_.regs);
    iov.iov_len = sizeof(data_.regs);
    if (ptrace(PTRACE_SETREGSET, pid_, NT_PRSTATUS, &iov) < 0)
    {
        Error::send_errno("Failed to read general purpose registers");
    }

    iov.iov_base = &(data_.fp_regs);
    iov.iov_len = sizeof(data_.fp_regs);
    if (ptrace(PTRACE_SETREGSET, pid_, NT_FPREGSET, &iov) < 0)
    {
        Error::send_errno("Failed to read vector registers");
    }
}

static RegisterValue
read_commmon(const RegisterInfo *info, std::uint8_t *src_addr)
{
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
            throw std::logic_error(
                "Register "+ std::string(info->name) + " has invalid size");
            break;
        }
    }
    
}

RegisterValue Process::read_register(std::string_view reg_name)
{
    const RegisterInfo *info = get_register_info(reg_name);

    std::uint8_t *src_addr = nullptr;
    if (info->type == RegisterType::RegGPR)
        src_addr = reinterpret_cast<std::uint8_t *>(&(data_.regs));
    else if (info->type == RegisterType::RegFPR)
        src_addr = reinterpret_cast<std::uint8_t *>(&(data_.fp_regs));

    src_addr += info->offset;

    return read_commmon(info, src_addr);
}

RegisterValue Process::read_register(RegisterID reg_id)
{
    const RegisterInfo *info = get_register_info(reg_id);

    std::uint8_t *src_addr = nullptr;
    if (info->type == RegisterType::RegGPR)
        src_addr = reinterpret_cast<std::uint8_t *>(&(data_.regs));
    else if (info->type == RegisterType::RegFPR)
        src_addr = reinterpret_cast<std::uint8_t *>(&(data_.fp_regs));

    src_addr += info->offset;
    return read_commmon(info, src_addr);
}

static RegisterValue
parse_register_token(std::string_view token, const RegisterInfo* info)
{
    if (token.find('.') != std::string_view::npos)
    {
        __uint128_t result = 0;
        
        if (token.back() == 'f' || token.back() == 'F')
        {
            if (info->size < 4)
                throw std::runtime_error("Register too small for float");
            
            float f_val = std::stof(std::string(token.substr(0, token.size() - 1)));
            std::memcpy(&result, &f_val, sizeof(float));
        }
        else
        {
            if (info->size < 8)
                throw std::runtime_error("Register too small for double");
            
            double d_val = std::stod(std::string(token));
            std::memcpy(&result, &d_val, sizeof(double));
        }
        
        if (info->size == 16) return result;
        if (info->size == 8)  return static_cast<uint64_t>(result);
        if (info->size == 4)  return static_cast<uint32_t>(result);
    }

    uint64_t val = 0;
    std::string s_token(token);
    bool is_hex = (token.size() > 2 && token[0] == '0' && (token[1] == 'x' || token[1] == 'X'));

    try
    {
        if (is_hex)
        {
            val = std::stoull(s_token, nullptr, 16);
        }
        else
        {
            val = std::stoull(s_token, nullptr, 10);
        }
    }
    catch (...)
    {
        throw std::runtime_error("Invalid numeric token: " + s_token);
    }

    switch (info->size)
    {
        case 1:
            if (val > UINT8_MAX)
                throw std::runtime_error("Value too large for 8-bit register");
            return static_cast<uint8_t>(val);
        case 2:
            if (val > UINT16_MAX)
                throw std::runtime_error("Value too large for 16-bit register");
            return static_cast<uint16_t>(val);
        case 4:
            if (val > UINT32_MAX)
                throw std::runtime_error("Value too large for 32-bit register");
            return static_cast<uint32_t>(val);
        case 8:
            return val;
        case 16:
            return static_cast<__uint128_t>(val);
        default:
            throw std::runtime_error("Unsupported register size");
    }
}

void Process::write_register(std::string_view reg_name, std::string_view val_str)
{
    const RegisterInfo *info = get_register_info(reg_name);

    std::uint8_t *dest_addr = nullptr;
    if (info->type == RegisterType::RegGPR)
        dest_addr = reinterpret_cast<std::uint8_t *>(&(data_.regs));
    else if (info->type == RegisterType::RegFPR)
        dest_addr = reinterpret_cast<std::uint8_t *>(&(data_.fp_regs));
    dest_addr += info->offset;

    RegisterValue val = parse_register_token(val_str, info);

    std::visit(
        [&](auto &&arg)
        {
            using T = std::decay_t<decltype(arg)>;

            if (sizeof(T) < info->size)
            {
                throw std::invalid_argument("Size mismatch for register " + std::string(reg_name));
            }

            std::memcpy(dest_addr, &arg, sizeof(T));
        },
        val);
}

void Process::write_register(std::string_view reg_name, RegisterValue val)
{
    const RegisterInfo *info = get_register_info(reg_name);

    std::uint8_t *dest_addr = nullptr;
    if (info->type == RegisterType::RegGPR)
        dest_addr = reinterpret_cast<std::uint8_t *>(&(data_.regs));
    else if (info->type == RegisterType::RegFPR)
        dest_addr = reinterpret_cast<std::uint8_t *>(&(data_.fp_regs));
    dest_addr += info->offset;

    std::visit(
        [&](auto &&arg)
        {
            using T = std::decay_t<decltype(arg)>;

            if (sizeof(T) < info->size)
            {
                throw std::invalid_argument("Size mismatch for register " + std::string(reg_name));
            }

            std::memcpy(dest_addr, &arg, sizeof(T));
        },
        val);
}
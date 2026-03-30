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

#include <csignal>
#include <cerrno>
#include <regex>
#include <fstream>
#include <string>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <optional>
#include <filesystem>

#include <sys/wait.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>

#include "error.hpp"

#define BUF_SIZE 256

using virt_addr = std::uint64_t;

bool process_exists(pid_t pid)
{
    if (pid <= 0)
        return false;

    // kill(pid, 0) checks if the process exists
    // without actually sending a signal
    if (kill(pid, 0) == 0)
        return true;

    // If it failed but errno is EPERM
    // the process exists but we don't own it
    if (errno == EPERM)
        return true;

    return false;
}

/*
|----------------------------------------------------------------------------|
| State | Name         | Meaning                                             |
| ----- | ------------ | --------------------------------------------------- |
|   R   | Running      | Currently running **or runnable** (on run queue)    |
|   S   | Sleeping     | Interruptible sleep (waiting for event, signalable) |
|   D   | Disk sleep   | Uninterruptible sleep (usually I/O)                 |
|   T   | Stopped      | Stopped by job control (`SIGSTOP`, `SIGTSTP`)       |
|   t   | Tracing stop | Stopped by debugger (`ptrace`)                      |
|   Z   | Zombie       | Exited, parent hasn’t `wait()`ed                    |
|   X   | Dead         | Being removed (rarely seen)                         |
|   I   | Idle         | Idle kernel thread                                  |
|----------------------------------------------------------------------------|
*/
char process_status(pid_t pid)
{
    if (pid <= 0)
        return 'N';

    std::string path = "/proc/" + std::to_string(pid) + "/stat";
    std::ifstream stat_file(path);

    if (!stat_file.is_open())
    {
        return 'N'; // Process likely exited between checks
    }

    std::string line;
    if (!std::getline(stat_file, line))
    {
        return 'N';
    }

    // The format is: pid (comm) state ...
    // The comm (command name) can have spaces, so we find the LAST ')'
    size_t last_paren = line.find_last_of(')');

    // The state char is 2 positions after the last ')'
    if (last_paren != std::string::npos &&
        (last_paren + 2) < line.size())
    {
        return line[last_paren + 2];
    }

    return 'N';
}

bool process_running(pid_t pid)
{
    char state = process_status(pid);
    switch (state)
    {
        case 'R': // Running
        case 'S': // Sleeping
        case 'D': // Disk Sleep
        case 'I': // Idle (Kernel)
            return true;
        default:
            return false;
    }
}

void process_destroy(pid_t pid)
{
    int status;
    kill(pid, SIGKILL);
    waitpid(pid, &status, 0);
}

pid_t process_create(char *const argv[],
    std::optional<int*> comm = std::nullopt)
{
    if (argv == nullptr || argv[0] == nullptr)
        return -1;

    int pipe_fd[2];
    if (::pipe2(pipe_fd, O_CLOEXEC) < 0) {
        ::perror("process_create pipe2");
        return -1;
    }

    int sv[2] = {-1, -1};
    if (comm && ::socketpair(AF_UNIX, SOCK_DGRAM, 0, sv) < 0)
    {
        ::perror("process_create socketpair");
        return -1;
    }

    pid_t debug_pid = ::fork();
    if (debug_pid < 0) {
        ::perror("process_create fork");
        ::close(pipe_fd[0]);
        ::close(pipe_fd[1]);
        if (comm)
        {
            ::close(sv[0]);
            ::close(sv[1]);
        }
        return -1;
    }

    if (debug_pid == 0)
    {
        ::close(pipe_fd[0]);

        if (comm)
        {
            // close parent's end
            ::close(sv[0]);

            // redirect all three standard streams to socket
            ::dup2(sv[1], STDIN_FILENO);
            ::dup2(sv[1], STDOUT_FILENO);
            ::dup2(sv[1], STDERR_FILENO);
    
            // original sv[1] no longer needed — stdin/out/err cover it now
            ::close(sv[1]);
        }

        std::string path = argv[0];
        
        if (path.find('/') == std::string::npos)
        {
            std::string local_path = "./" + path;
            if (::access(local_path.c_str(), X_OK) == 0)
            {
                path = local_path;
            }
        }

        ::execvp(path.c_str(), argv);

        std::string err_msg = "process_create execvp failed for " + path + ": " + std::strerror(errno);
        ::write(pipe_fd[1], err_msg.c_str(), err_msg.size());
        ::close(pipe_fd[1]);
        ::_exit(1);
    }

    // --- PARENT ---
    ::close(pipe_fd[1]);
    if (comm) ::close(sv[1]);  // close child's end

    char buf[256];
    ssize_t b_read = ::read(pipe_fd[0], buf, sizeof(buf) - 1);
    ::close(pipe_fd[0]);

    if (b_read > 0)
    {
        buf[b_read] = '\0';
        ::waitpid(debug_pid, nullptr, 0);
        std::cerr << buf << std::endl;
        if(comm) ::close(sv[0]);
        return -1;
    }

    if (comm) **comm = sv[0];

    return debug_pid;
}

std::int64_t get_section_load_bias(
    std::filesystem::path path, Elf64_Addr virt_address)
{
    auto command = std::string("readelf -WS ") + path.string();
    auto pipe = popen(command.c_str(), "r");

    std::regex text_regex(R"(PROGBITS\s+(\w+)\s+(\w+)\s+(\w+))");
    char *line = nullptr;
    std::size_t len = 0;

    while (getline(&line, &len, pipe) != -1)
    {
        std::cmatch groups;
        if (std::regex_search(line, groups, text_regex))
        {
            auto start_addr = std::stol(groups[1], nullptr, 16);
            auto offset = std::stol(groups[2], nullptr, 16);
            auto size = std::stol(groups[3], nullptr, 16);
            if (start_addr <= virt_address &&
                virt_address < (start_addr + size))
            {
                free(line);
                pclose(pipe);
                return start_addr - offset;
            }
        }
        free(line);
        line = nullptr;
    }
    pclose(pipe);
    throw std::runtime_error("Could not section load bias");
}

std::int64_t get_entry_point_offset(std::filesystem::path path)
{
    std::ifstream elf_file(path);
    Elf64_Ehdr header;

    elf_file.read(reinterpret_cast<char*> (&header), sizeof(header));
    Elf64_Addr entry_address = header.e_entry;
    auto bias = get_section_load_bias(path, entry_address);
    return entry_address - bias;
}

virt_addr get_load_address(pid_t pid, virt_addr inst_offset)
{
    std::ifstream maps("/proc/" + std::to_string(pid) + "/maps");
    std::regex map_regex(R"((\w+)-\w+ ..(.). (\w+))");
    std::string line;
    
    while (std::getline(maps, line))
    {
        std::smatch groups;
        std::regex_search(line, groups, map_regex);

        if (groups[2] == "x")
        {
            // Load base of the .text segment into virtual memory.
            // file_ offset from    
            virt_addr start_addr  = std::stoull(groups[1], nullptr, 16);
            uint64_t  file_offset = std::stoull(groups[3], nullptr, 16);

            return start_addr + inst_offset - file_offset;
        }
    }
    throw std::runtime_error("Could not find load address");
}

void print_vec(std::vector<std::uint32_t> &res, virt_addr start)
{
    virt_addr offset = start;
    std::cout << "____________________________________\n";
    std::cout << std::hex << std::showbase;
    for(std::size_t i = 0; i < res.size(); i++)
    {
        
        std::cout << offset << ": " << res[i] << std::endl;
        offset += 4;
    }
    std::cout << std::dec << std::noshowbase;
    std::cout << "____________________________________\n"; 

}

void read_from_socket(int fd, std::string &buf)
{
    buf.resize(BUF_SIZE);
    ssize_t bytes_read = recv(fd, buf.data(), BUF_SIZE, MSG_DONTWAIT);
    
    if (bytes_read < 0)
    {
        Error::send_errno("Socket read failed");
    }

    buf.resize(static_cast<std::size_t>(bytes_read));
}
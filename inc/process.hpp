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
#include <vector>
#include <string_view>

enum class ProcessState
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
    launch(std::vector<std::string_view> &exec_args);

    static std::unique_ptr<Process>
    attach(pid_t pid);

    std::uint8_t wait();
    void resume();

    pid_t get_pid() { return pid_; }
    ProcessState get_state() { return state_; }

private:
    Process(pid_t pid, bool kill_on_end) : pid_(pid), kill_on_end_(kill_on_end) {}

    pid_t pid_ = 0;
    bool kill_on_end_ = true;
    ProcessState state_ = ProcessState::Init;
};

#endif

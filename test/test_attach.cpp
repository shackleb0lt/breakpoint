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

#include <catch2/catch_test_macros.hpp>

#include "test_common.hpp"
#include "process.hpp"
#include <unistd.h>

// Mock utility to find a non-existent PID
pid_t get_unused_pid()
{
    pid_t unused_pid = 999999;

    while (process_exists(unused_pid) == true && unused_pid > 0)
        unused_pid--;

    return unused_pid;
}

TEST_CASE("Process Attachment Logic 1")
{
    SECTION("Attaching to a non-existent PID should throw")
    {
        pid_t bad_pid = get_unused_pid();
        CHECK_THROWS_AS(Process::attach(bad_pid), Error);
    }

    SECTION("Insufficient permissions when attaching to PID 1")
    {
        if (process_exists(1))
        {
            CHECK_THROWS_AS(Process::attach(1), Error);
        }
    }
}

TEST_CASE("Process Attachment Logic 2")
{
    char *const exec[] = 
    {
        (char *)"two_seconds",
        nullptr
    };

    pid_t debug_pid = process_create(exec);

    REQUIRE(process_exists(debug_pid));
    CHECK(process_running(debug_pid));

    // Attach to the process
    auto proc = Process::attach(debug_pid);
    REQUIRE(proc != nullptr);

    // 't' stands for Tracing Stop
    CHECK(process_status(proc->get_pid()) == 't');
    CHECK(proc->get_state() == ProcessState::Stopped);

    SECTION("Full lifecycle: Launch, Attach, Resume, and Wait")
    {
        // Resume the process
        proc->resume();
        CHECK(process_running(debug_pid));
        CHECK(proc->get_state() == ProcessState::Running);

        // Wait for finish
        proc->wait();
        CHECK_FALSE(process_exists(debug_pid));
        CHECK(proc->get_state() == ProcessState::Exited);
    }

    SECTION("Detaching from a Running Process")
    {
        proc->resume();
        CHECK(process_running(debug_pid));
        CHECK(proc->get_state() == ProcessState::Running);

        // Call destructor,
        // Essentially detaching from process
        proc.reset();

        // After detach, process should still exist and run independently
        CHECK(process_exists(debug_pid));
        CHECK(process_running(debug_pid));

        // Cleanup
        process_destroy(debug_pid);
    }

    SECTION("Detaching from a Stopped Process")
    {
        proc.reset();

        CHECK(process_exists(debug_pid));
        CHECK(process_running(debug_pid));

        // Cleanup
        process_destroy(debug_pid);
    }
}

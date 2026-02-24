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

TEST_CASE("Process Launch Logic 1")
{
    SECTION("Empty exec args should throw")
    {
        std::vector<std::string_view> empty;
        CHECK_THROWS_AS(Process::launch(empty), Error);
    }

    SECTION("Launching invalid binary should throw")
    {
        std::vector<std::string_view> fake = {"voldemort"};
        CHECK_THROWS_AS(Process::launch(fake), Error);
    }
}

TEST_CASE("Process Launch Logic 2")
{
    std::vector<std::string_view> exec = 
    {
        "two_seconds"
    };

    auto proc = Process::launch(exec);
    REQUIRE(proc != nullptr);

    pid_t pid = proc->get_pid();

    REQUIRE(process_exists(pid));
    CHECK(process_status(pid) == 't');
    CHECK(proc->get_state() == ProcessState::Stopped);

    SECTION("Normal execution to completion")
    {
        // Resume the process
        proc->resume();
        CHECK(process_running(pid));
        CHECK(proc->get_state() == ProcessState::Running);

        // Wait for finish
        proc->wait();
        CHECK(proc->get_state() == ProcessState::Exited);

        proc.reset();
        CHECK_FALSE(process_exists(pid));
    }

    SECTION("Cleanup while process is running")
    {
        proc->resume();
        CHECK(proc->get_state() == ProcessState::Running);

        proc.reset();
        CHECK_FALSE(process_exists(pid));
    }

    SECTION("Cleanup while process is stopped")
    {
        CHECK(proc->get_state() == ProcessState::Stopped);

        proc.reset();
        CHECK_FALSE(process_exists(pid));
    }
}

TEST_CASE("Fast Exiting Process")
{
    std::vector<std::string_view> exec =
    {
        "outta_here"
    };

    auto proc = Process::launch(exec);
    REQUIRE(proc != nullptr);

    pid_t pid = proc->get_pid();

    REQUIRE(process_exists(pid));
    CHECK(process_status(pid) == 't');
    CHECK(proc->get_state() == ProcessState::Stopped);

    SECTION("Process that exits immediately after resume")
    {
        proc->resume();
        // Even if it exits fast, the state logic should track it
        proc->wait();
        CHECK(proc->get_state() == ProcessState::Exited);

        proc.reset();
        CHECK_FALSE(process_exists(pid));
    }
}
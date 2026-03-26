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

#include "process.hpp"
#include "test_common.hpp"

TEST_CASE("Read Write Memory Directly")
{
    std::vector<std::string_view> exec =
    {
        "memory"
    };

    int sockfd = -1;
    auto proc = Process::launch(exec, &sockfd);
    REQUIRE(proc != nullptr);

    pid_t pid = proc->get_pid();
    REQUIRE(process_exists(pid));
    CHECK(process_status(pid) == 't');
    CHECK(proc->get_state() == ProcessState::Stopped);

    proc->resume();

    uint8_t info = proc->wait();
    CHECK(process_status(pid) == 't');
    CHECK(proc->get_state() == ProcessState::Stopped);
    CHECK(info == SIGTRAP);

    std::string output;
    read_from_socket(sockfd, output);
    CHECK(output.size() > 0);

    virt_addr ptr;
    std::memcpy(&ptr, output.data(), sizeof(virt_addr));

    uint64_t data = proc->read_memory_as<uint64_t>(ptr);
    CHECK(data == 0xcafecafedeaddead);

    proc->resume();

    info = proc->wait();
    CHECK(process_status(pid) == 't');
    CHECK(proc->get_state() == ProcessState::Stopped);
    CHECK(info == SIGTRAP);

    output.clear();
    read_from_socket(sockfd, output);
    CHECK(output.size() > 0);

    std::memcpy(&ptr, output.data(), sizeof(virt_addr));

    const std::uint8_t out[16] = "Hello World!";
    proc->write_memory(ptr, {out, 13});

    proc->resume();

    std::uint8_t ret = proc->wait();
    CHECK(proc->get_state() == ProcessState::Exited);
    CHECK(ret == 0);

    output.clear();
    read_from_socket(sockfd, output);

    CHECK(output == "Hello World!");
    close(sockfd);
}

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

TEST_CASE("Breakpoint Site Testing")
{
    std::vector<std::string_view> exec = 
    {
        "outta_here"
    };

    auto proc = Process::launch(exec);
    const auto& cproc = proc;
    REQUIRE(proc != nullptr);

    pid_t pid = proc->get_pid();

    REQUIRE(process_exists(pid));
    CHECK(process_status(pid) == 't');
    CHECK(proc->get_state() == ProcessState::Stopped);

    SECTION("Creation of breakpoint")
    {
        BreakpointSite &site = proc->create_breakpoint_site(0xaaaa);
        REQUIRE(site.address() == 0xaaaa);
        CHECK_THROWS_AS(proc->create_breakpoint_site(0xaaaa), Error);
    }
 
    SECTION("Type ID increments")
    {
        auto& s1 = proc->create_breakpoint_site(0xaaaa);
        REQUIRE(s1.address() == 0xaaaa);

        auto& s2 = proc->create_breakpoint_site(0xbbbb);
        REQUIRE(s2.id() == s1.id() + 1);

        auto& s3 = proc->create_breakpoint_site(0xcccc);
        REQUIRE(s3.id() == s2.id() + 1);
        REQUIRE(s3.address() == 0xcccc);

        auto& s4 = proc->create_breakpoint_site(0xdddd);
        REQUIRE(s4.id() == s3.id() + 1);
    }

    SECTION("Finding breakpoints by id and address")
    {
        proc->create_breakpoint_site(0xaaaa);
        proc->create_breakpoint_site(0xbbbb);
        proc->create_breakpoint_site(0xcccc);
        proc->create_breakpoint_site(0xdddd);

        auto& s1 = proc->breakpoint_sites().get_by_address(0xcccc);
        REQUIRE(proc->breakpoint_sites().contains_address(0xcccc));
        REQUIRE(s1.address()== 0xcccc);

        auto& cs1 = cproc->breakpoint_sites().get_by_address(0xcccc);
        REQUIRE(cproc->breakpoint_sites().contains_address(0xcccc));
        REQUIRE(cs1.address() == 0xcccc);

        auto& s2 = proc->breakpoint_sites().get_by_id(s1.id() + 1);
        REQUIRE(proc->breakpoint_sites().contains_id(s1.id() + 1));
        REQUIRE(s2.id() == s1.id() + 1);
        REQUIRE(s2.address() == 0xdddd);

        auto& cs2 = proc->breakpoint_sites().get_by_id(cs1.id() + 1);
        REQUIRE(cproc->breakpoint_sites().contains_id(cs1.id() + 1));
        REQUIRE(cs2.id() == cs1.id() + 1);
        REQUIRE(cs2.address() == 0xdddd);
    }

    SECTION ("Cannot find breakpoint site")
    {
        REQUIRE_THROWS_AS(proc->breakpoint_sites().get_by_address(0xdddd), Error);
        REQUIRE_THROWS_AS(proc->breakpoint_sites().get_by_id(0xdddd), Error);
        REQUIRE_THROWS_AS(cproc->breakpoint_sites().get_by_address(0xdddd), Error);
        REQUIRE_THROWS_AS(cproc->breakpoint_sites().get_by_id(0xdddd), Error);
    }

    SECTION("Breakpoint site list size and emptiness")
    {
        REQUIRE(proc->breakpoint_sites().empty());
        REQUIRE(proc->breakpoint_sites().size() == 0);
        REQUIRE(cproc->breakpoint_sites().empty());
        REQUIRE(cproc->breakpoint_sites().size() == 0);

        proc->create_breakpoint_site(0xaaaa);
        REQUIRE(!proc->breakpoint_sites().empty());
        REQUIRE(proc->breakpoint_sites().size() == 1);
        REQUIRE(!cproc->breakpoint_sites().empty());
        REQUIRE(cproc->breakpoint_sites().size() == 1);

        proc->create_breakpoint_site(0xbbbb);
        REQUIRE(!proc->breakpoint_sites().empty());
        REQUIRE(proc->breakpoint_sites().size() == 2);
        REQUIRE(!cproc->breakpoint_sites().empty());
        REQUIRE(cproc->breakpoint_sites().size() == 2);
    }

    SECTION("Breakpoint sites iteration")
    {
        proc->create_breakpoint_site(0xaaaa);
        proc->create_breakpoint_site(0xaaab);
        proc->create_breakpoint_site(0xaaac);
        proc->create_breakpoint_site(0xaaad);

        proc->breakpoint_sites().for_each(
            [addr = 0xaaaa](auto& site) mutable {
                REQUIRE(site.address() == addr++);
            });

        cproc->breakpoint_sites().for_each(
            [addr = 0xaaaa](auto& site) mutable {
                REQUIRE(site.address() == addr++);
            });
    }

    proc.reset();
    CHECK_FALSE(process_exists(pid));
}

TEST_CASE("Breakpoint Enable and Disable")
{
    std::vector<std::string_view> exec = 
    {
        "hello"
    };

    auto proc = Process::launch(exec);
    REQUIRE(proc != nullptr);

    pid_t pid = proc->get_pid();

    REQUIRE(process_exists(pid));
    CHECK(process_status(pid) == 't');
    CHECK(proc->get_state() == ProcessState::Stopped);

    SECTION("Enable breakpoint")
    {
        auto offset = get_entry_point_offset("hello");
        auto load_address = get_load_address(pid, offset);
        proc->create_breakpoint_site(load_address).enable();
        proc->resume();

        auto reason = proc->wait();
        CHECK(process_status(pid) == 't');
        CHECK(proc->get_state() == ProcessState::Stopped);
        REQUIRE(reason == SIGTRAP);

        REQUIRE(proc->get_pc() == load_address);
        proc->resume();
    }

    SECTION("Disable breakpoint")
    {
        auto offset = get_entry_point_offset("hello");
        auto load_address = get_load_address(pid, offset);
        BreakpointSite &bp = proc->create_breakpoint_site(load_address);
        bp.enable();
        bp.disable();
        proc->resume();
    }

    auto reason = proc->wait();
    CHECK(proc->get_state() == ProcessState::Exited);   
    REQUIRE(reason == 0);

    CHECK_FALSE(process_exists(pid));
}

TEST_CASE("Hardware Breakpoint Testing")
{
    std::vector<std::string_view> exec = 
    {
        "anti_gdb"
    };

    int sockfd = -1;
    auto proc = Process::launch(exec, &sockfd);
    REQUIRE(proc != nullptr);

    pid_t pid = proc->get_pid();
    REQUIRE(process_exists(pid));
    CHECK(process_status(pid) == 't');
    CHECK(proc->get_state() == ProcessState::Stopped);

    proc->resume();

    // Read innocent function address
    uint8_t info = proc->wait();
    CHECK(process_status(pid) == 't');
    CHECK(proc->get_state() == ProcessState::Stopped);
    CHECK(info == SIGTRAP);

    std::string output;
    read_from_socket(sockfd, output);
    CHECK(output.size() > 0);

    virt_addr ptr;
    std::memcpy(&ptr, output.data(), sizeof(virt_addr));

    auto &soft = proc->create_breakpoint_site(ptr, false);
    soft.enable();

    proc->resume();
    proc->wait();

    output.clear();
    read_from_socket(sockfd, output);
    CHECK(output == "Putting pepporoni on pizza...\n");

    proc->breakpoint_sites().remove_by_id(soft.id());

    CHECK(proc->breakpoint_sites().empty() == true);
    
    auto &hard = proc->create_breakpoint_site(ptr, true);
    hard.enable();

    proc->resume();
    proc->wait();

    CHECK(proc->get_pc() == ptr);
    proc->resume();
    proc->wait();

    output.clear();
    read_from_socket(sockfd, output);
    CHECK(output == "Putting pineapple on pizza...\n");

    proc.reset();

    CHECK_FALSE(process_exists(pid));
}


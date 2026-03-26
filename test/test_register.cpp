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

#include <cstdint>
#include <string>
#include <vector>

#include "process.hpp"
#include "test_common.hpp"

static uint64_t u64_from(RegisterValue const& v)
{
    if (std::holds_alternative<uint64_t>(v))
        return std::get<uint64_t>(v);
    else if (std::holds_alternative<double>(v))
    {
        std::uint64_t res = 0;
        std::memcpy(&res, &v, sizeof(res));
        return res;
    }
    return 0;
}

static uint32_t u32_from(RegisterValue const& v)
{
    if (std::holds_alternative<uint32_t>(v))
    {
        return std::get<uint32_t>(v);
    }
    else if (std::holds_alternative<float>(v))
    {
        std::uint32_t res = 0;
        std::memcpy(&res, &v, sizeof(res));
        return res;
    }
    return 0;
}

static uint16_t u16_from(RegisterValue const& v)
{
    if (std::holds_alternative<uint16_t>(v))
        return std::get<uint16_t>(v);
    return 0;
}

static uint8_t u8_from(RegisterValue const& v)
{
    if (std::holds_alternative<uint8_t>(v))
        return std::get<uint8_t>(v);
    return 0;
}

static double dbl_from(RegisterValue const& v)
{
    if (std::holds_alternative<double>(v))
        return std::get<double>(v);
    return 0.0;
}

static float flt_from(RegisterValue const& v)
{
    if (std::holds_alternative<float>(v))
        return std::get<float>(v);
    return 0.0f;
}

TEST_CASE("Read all registers")
{
    std::vector<std::string_view> exec =
    {
        "outta_here",
        "dummy"
    };

    auto proc = Process::launch(exec);
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

    virt_addr addr;
    CHECK_NOTHROW(addr = proc->get_pc());
    CHECK_NOTHROW(proc->set_pc(0xFFFFFFFFULL));

    CHECK(proc->get_pc() == 0xFFFFFFFFULL);
    CHECK_NOTHROW(proc->set_pc(addr));

    std::size_t end = static_cast<std::size_t>(RegisterID::REG32_FPCR);
    for (std::size_t curr = 0;  curr <= end; curr++)
    {
        RegisterID id = static_cast<RegisterID>(curr);
        CHECK_NOTHROW(proc->registers().read<RegisterValue>(id));
    }

    proc->resume();
    std::uint8_t ret = proc->wait();
    CHECK(proc->get_state() == ProcessState::Exited);
    CHECK(ret == 0);
}

TEST_CASE("Register read via asm_magic")
{
    std::vector<std::string_view> exec =
    {
        "asm_magic",
        "r",
        "0xcafebeefdeadbabe",
        "0xbeadfade",
        "0xface",
        "0xbe",
        "0x1.a2b3c4d5e6f78p+5",
        "0x1.9f7e5dp-3f"
    };

    std::uint64_t val64 = 0xcafebeefdeadbabe;
    std::uint32_t val32 = 0xbeadfade;
    std::uint16_t val16 = 0xface;
    std::uint8_t val8 = 0xbe;

    double valD = 0x1.a2b3c4d5e6f78p+5;
    float valF = 0x1.9f7e5dp-3f;

    auto proc = Process::launch(exec);
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

    SECTION("read using string_view")
    {
        #if defined(__aarch64__)

        CHECK_THROWS_AS(proc->registers().read<RegisterValue>("x34"), std::invalid_argument);

        CHECK(proc->registers().read<uint64_t>("x19") == val64);
        CHECK(proc->registers().read<uint32_t>("w20") == val32);

        CHECK(proc->registers().read<double>("d19") == valD);
        CHECK(proc->registers().read<float>("s20")  == valF);

        CHECK(proc->registers().read<uint8_t>("b21") == val8);
        CHECK(proc->registers().read<uint16_t>("h22") == val16);
        CHECK(proc->registers().read<uint32_t>("s23") == val32);
        CHECK(proc->registers().read<uint64_t>("d24") == val64);
        #endif
    }

    SECTION("read using RegisterID")
    {
        #if defined(__aarch64__)
        CHECK(proc->registers().read<uint64_t>(RegisterID::REG64_X19) == val64);
        CHECK(proc->registers().read<uint32_t>(RegisterID::REG32_W20) == val32);

        CHECK(proc->registers().read<double>(RegisterID::REG64_D19) == valD);
        CHECK(proc->registers().read<float>(RegisterID::REG32_S20)  == valF);

        CHECK(proc->registers().read<uint8_t>(RegisterID::REG8_B21)   == val8);
        CHECK(proc->registers().read<uint16_t>(RegisterID::REG16_H22) == val16);
        CHECK(proc->registers().read<uint32_t>(RegisterID::REG32_S23) == val32);
        CHECK(proc->registers().read<uint64_t>(RegisterID::REG64_D24) == val64);
        #endif
    }

    proc->resume();
    std::uint8_t ret = proc->wait();
    CHECK(proc->get_state() == ProcessState::Exited);
    CHECK(ret == 0);
}

TEST_CASE("Register write with RegisterValue via asm_magic")
{
    std::vector<std::string_view> exec =
    {
        "asm_magic",
        "w",
        "0xcafebeefdeadbabe",
        "0xbeadfade",
        "0xface",
        "0xbe",
        "0x1.a2b3c4d5e6f78p+5",
        "0x1.9f7e5dp-3f"
    };

    std::uint64_t val64 = 0xcafebeefdeadbabe;
    std::uint32_t val32 = 0xbeadfade;
    std::uint16_t val16 = 0xface;
    std::uint8_t val8 = 0xbe;

    double valD = 0x1.a2b3c4d5e6f78p+5;
    float valF = 0x1.9f7e5dp-3f;

    auto proc = Process::launch(exec);
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

    SECTION("Write value via RegisterValue")
    {    
        #if defined(__aarch64__)
        proc->registers().write("x9",  RegisterValue{val64});
        proc->registers().write("w10", RegisterValue{val32});
        proc->registers().write("x11", RegisterValue{val32});
        proc->registers().write("d19", RegisterValue{valD});
        proc->registers().write("s20", RegisterValue{valF});
        proc->registers().write("b21", RegisterValue{val8});
        proc->registers().write("h22", RegisterValue{val16});
        proc->registers().write("s23", RegisterValue{val32});
        proc->registers().write("d24", RegisterValue{val64});
        #endif
    }

    SECTION("Write value via string_view")
    {    
        #if defined(__aarch64__)
        proc->registers().write("x9",  "0xcafebeefdeadbabe");
        proc->registers().write("w10", "0xbeadfade");
        proc->registers().write("x11", "0xbeadfade");
        proc->registers().write("d19", "0x1.a2b3c4d5e6f78p+5");
        proc->registers().write("s20", "0x1.9f7e5dp-3f");
        proc->registers().write("b21", "0xbe");
        proc->registers().write("h22", "0xface");
        proc->registers().write("s23", "0xbeadfade");
        proc->registers().write("d24", "0xcafebeefdeadbabe");
        #endif
    }

    proc->resume();
    std::uint8_t ret = proc->wait();
    CHECK(proc->get_state() == ProcessState::Exited);
    CHECK(ret == 0);
}

TEST_CASE("Program Counter invalid acccess")
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

    proc->resume();
    CHECK_THROWS_AS(proc->get_pc(), Error);
    CHECK_THROWS_AS(proc->set_pc(0xCAFEEFACFADEEDAF), Error);

    proc.reset();
    CHECK_FALSE(process_exists(pid));
}

/*
asm_magic w 0xcafebeefdeadbabe 0xbeadfade 0xface 0xbe 0x1.a2b3c4d5e6f78p+5 0x1.9f7e5dp-3f
asm_magic r 0xcafebeefdeadbabe 0xbeadfade 0xface 0xbe 0x1.a2b3c4d5e6f78p+5 0x1.9f7e5dp-3f
*/

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
    CHECK_NOTHROW(proc->set_pc(addr));

    std::size_t end = static_cast<std::size_t>(RegisterID::REG32_FPCR);
    for (std::size_t curr = 0;  curr <= end; curr++)
    {
        RegisterID id = static_cast<RegisterID>(curr);
        CHECK_NOTHROW(proc->read_register(id));
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
        CHECK_THROWS_AS(proc->read_register("x34"), std::invalid_argument);

        CHECK(u64_from(proc->read_register("x19")) == val64);
        CHECK(u32_from(proc->read_register("w20")) == val32);

        CHECK(dbl_from(proc->read_register("d19")) == valD);
        CHECK(flt_from(proc->read_register("s20")) == valF);

        CHECK(u8_from(proc->read_register("b21"))  == val8);
        CHECK(u16_from(proc->read_register("h22")) == val16);
        CHECK(u32_from(proc->read_register("s23")) == val32);
        CHECK(u64_from(proc->read_register("d24")) == val64);
        #endif
    }

    SECTION("read using RegisterID")
    {
        #if defined(__aarch64__)
        CHECK(u64_from(proc->read_register(RegisterID::REG64_X19)) == val64);
        CHECK(u32_from(proc->read_register(RegisterID::REG32_W20)) == val32);

        CHECK(dbl_from(proc->read_register(RegisterID::REG64_D19)) == valD);
        CHECK(flt_from(proc->read_register(RegisterID::REG32_S20)) == valF);

        CHECK(u8_from(proc->read_register(RegisterID::REG8_B21))   == val8);
        CHECK(u16_from(proc->read_register(RegisterID::REG16_H22)) == val16);
        CHECK(u32_from(proc->read_register(RegisterID::REG32_S23)) == val32);
        CHECK(u64_from(proc->read_register(RegisterID::REG64_D24)) == val64);
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
#if defined(__aarch64__)
    proc->write_register("x9",  RegisterValue{val64});
    proc->write_register("w10", RegisterValue{val32});
    proc->write_register("x11", RegisterValue{val32});
    proc->write_register("d19", RegisterValue{valD});
    proc->write_register("s20", RegisterValue{valF});
    proc->write_register("b21", RegisterValue{val8});
    proc->write_register("h22", RegisterValue{val16});
    proc->write_register("s23", RegisterValue{val32});
    proc->write_register("d24", RegisterValue{val64});
#endif

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


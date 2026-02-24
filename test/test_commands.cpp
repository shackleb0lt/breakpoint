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
#include "commands.hpp"

TEST_CASE("process_line - top-level commands")
{
    SECTION("quit")
    {
        auto [action, tokens] = process_line("quit");
        REQUIRE(action == Action::Quit);
        REQUIRE(tokens.size() == 1);
        REQUIRE(tokens[0] == "quit");
    }

    SECTION("continue")
    {
        auto [action, tokens] = process_line("continue");
        REQUIRE(action == Action::Continue);
        REQUIRE(tokens[0] == "continue");
    }

    SECTION("help")
    {
        auto [action, tokens] = process_line("help");
        REQUIRE(action == Action::Help);
        REQUIRE(tokens[0] == "help");
    }

    SECTION("empty line yields None")
    {
        auto [action, tokens] = process_line("");
        REQUIRE(action == Action::None);
        REQUIRE(tokens.empty());
    }

    SECTION("whitespace-only yields None")
    {
        auto [action, tokens] = process_line("   \t  ");
        REQUIRE(action == Action::None);
        REQUIRE(tokens.empty());
    }
}

TEST_CASE("process_line - register subcommands")
{
    SECTION("register read")
    {
        auto [action, tokens] = process_line("register read");
        REQUIRE(action == Action::ReadRegGPR);
        REQUIRE(tokens.size() == 2);
        REQUIRE(tokens[0] == "register");
        REQUIRE(tokens[1] == "read");
    }

    SECTION("register read all")
    {
        auto [action, tokens] = process_line("register read all");
        REQUIRE(action == Action::ReadRegAll);
        REQUIRE(tokens.size() == 3);
    }

    SECTION("register read <gpr> is ReadReg")
    {
        auto [action, tokens] = process_line("register read rax");
        REQUIRE(action == Action::ReadReg);
        REQUIRE(tokens.size() == 3);
        REQUIRE(tokens[2] == "rax");
    }

    SECTION("register write is Incomplete")
    {
        auto [action, tokens] = process_line("register write");
        REQUIRE(action == Action::Incomplete);
    }

    SECTION("register write <name> is WriteReg")
    {
        auto [action, tokens] = process_line("register write rbx 0x42");
        REQUIRE(action == Action::WriteReg);
    }
}

TEST_CASE("process_line - invalid and ambiguous")
{
    SECTION("invalid command")
    {
        auto [action, tokens] = process_line("nonesuch");
        REQUIRE(action == Action::Invalid);
    }

    SECTION("ambiguous prefix - register read a matches 'all' and default")
    {
        auto [action, tokens] = process_line("register read a");
        REQUIRE(action == Action::Ambiguous);
    }

    SECTION("incomplete register")
    {
        auto [action, tokens] = process_line("register");
        REQUIRE(action == Action::Incomplete);
    }
}

TEST_CASE("process_line - tokenization")
{
    SECTION("leading and trailing whitespace ignored for command")
    {
        auto [action, tokens] = process_line("  continue  ");
        REQUIRE(action == Action::Continue);
        REQUIRE(tokens.size() == 1);
        REQUIRE(tokens[0] == "continue");
    }

    SECTION("multiple spaces between tokens")
    {
        auto [action, tokens] = process_line("register   read   all");
        REQUIRE(action == Action::ReadRegAll);
        REQUIRE(tokens.size() == 3);
    }
}
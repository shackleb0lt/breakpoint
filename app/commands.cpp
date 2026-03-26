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

#include "commands.hpp"

// Forward declarations for nested tables
extern const Command cmd_register[];
extern const Command cmd_register_read[];

const Command cmd_register_write_name[] = {
    {"",            Action::WriteReg,   nullptr},
    {"",            Action::Invalid,    nullptr} // Sentinel
};

const Command cmd_register_write[] = {
    {"",            Action::Incomplete, cmd_register_write_name},
    {"",            Action::Invalid,    nullptr}
};

const Command cmd_register_read[] = {
    {"all",         Action::ReadRegAll, nullptr},
    {"",            Action::ReadReg,    nullptr},
    {"",            Action::Invalid,    nullptr}
};

const Command cmd_register[] = {
    {"read",        Action::ReadRegGPR, cmd_register_read},
    {"write",       Action::Incomplete, cmd_register_write},
    {"",            Action::Invalid,    nullptr}
};

const Command cmd_memory_write_address[] = {
    {"",            Action::MemWrite, nullptr},
    {"",            Action::Invalid,    nullptr}
};

const Command cmd_memory_read_address[] = {
    {"",            Action::MemReadCnt, nullptr},
    {"",            Action::Invalid,    nullptr}
};

const Command cmd_memory_read[] = {
    {"",            Action::MemReadDef, cmd_memory_read_address},
    {"",            Action::Invalid,    nullptr}
};

const Command cmd_memory_write[] = {
    {"",            Action::Incomplete, cmd_memory_write_address},
    {"",            Action::Invalid,    nullptr}
};

const Command cmd_memory[] = {
    {"read",        Action::Incomplete, cmd_memory_read},
    {"write",       Action::Incomplete, cmd_memory_write},
    {"",            Action::Invalid,    nullptr}
};

const Command cmd_breakpoint_set[] = {
    {"",            Action::BPSiteSet,  nullptr},
    {"",            Action::Invalid,    nullptr}
};

const Command cmd_breakpoint_enable[] = {
    {"",            Action::BPSiteEn,   nullptr},
    {"",            Action::Invalid,    nullptr}
};

const Command cmd_breakpoint_disable[] = {
    {"",            Action::BPSiteDis,  nullptr},
    {"",            Action::Invalid,    nullptr}
};

const Command cmd_breakpoint_delete[] = {
    {"",            Action::BPSiteDel,  nullptr},
    {"",            Action::Invalid,    nullptr}
};

const Command cmd_breakpoint[] = {
    {"list",        Action::BPSiteList, nullptr},
    {"set",         Action::Incomplete, cmd_breakpoint_set},
    {"enable",      Action::Incomplete, cmd_breakpoint_enable},
    {"disable",     Action::Incomplete, cmd_breakpoint_disable},
    {"delete",      Action::Incomplete, cmd_breakpoint_delete},
    {"",            Action::Invalid,    nullptr}
};

const Command top_level[] = {
    {"breakpoint",  Action::Incomplete, cmd_breakpoint},
    {"continue",    Action::Continue,   nullptr},
    {"help",        Action::Help,       nullptr},
    {"memory",      Action::Incomplete, cmd_memory},
    {"register",    Action::Incomplete, cmd_register},
    {"quit",        Action::Quit,       nullptr},
    {"step",        Action::StepInst,   nullptr},
    {"",            Action::Invalid,    nullptr}
};

std::vector<std::string_view>
tokenize(std::string_view line)
{
    std::vector<std::string_view> tokens;
    tokens.reserve(8);
    size_t start = line.find_first_not_of(" \t\r\n");

    while (start != std::string_view::npos)
    {
        const size_t end = line.find_first_of(" \t\r\n", start);
        if (end == std::string_view::npos)
        {
            tokens.push_back(line.substr(start));
            break;
        }

        tokens.push_back(line.substr(start, end - start));
        start = line.find_first_not_of(" \t\r\n", end);
    }
    return tokens;
}

// Helper to find matches in a command list
std::pair<size_t, const Command*>
find_match(const Command* list, std::string_view token)
{
    size_t matches = 0;
    const Command* last_match = nullptr;

    for (const Command* curr = list; curr->action != Action::Invalid; ++curr)
    {
        // Exact match
        if (token == curr->keyword)
        {
            return {1, curr};
        }

        // Prefix match or default ("") match
        if (curr->keyword.empty() ||
            curr->keyword.substr(0, token.size()) == token)
        {
            last_match = curr;
            matches++;
        }
    }
    return {matches, last_match};
}

std::pair<Action, std::vector<std::string_view>>
process_line(std::string_view line)
{
    auto tokens = tokenize(line);
    if (tokens.empty())
        return {Action::None, tokens};

    // Start searching from top level table
    const Command *current_table = top_level;
    const Command *result_cmd = nullptr;

    for (const auto &token : tokens)
    {
        if (!current_table)
            break;

        auto [match_count, match_ptr] = find_match(current_table, token);

        if (match_count == 0)
            return {Action::Invalid, tokens};
        if (match_count > 1)
            return {Action::Ambiguous, tokens};

        result_cmd = match_ptr;
        current_table = result_cmd->children;
    }

    if (!result_cmd)
        return {Action::Invalid, tokens};

    return {result_cmd->action, tokens};
}
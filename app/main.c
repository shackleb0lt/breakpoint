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
#include "linenoise.h"
#include "process.h"
#include "commands.h"

#include <stdio.h>
#include <stdint.h>

process_t debug_proc = {0};

void display_variant(process_t *proc, const char *reg_name, variant_t *var)
{
    switch (var->type)
    {
        case TYPE_UINT8:
            printf("  %-12s 0x%-16x %-24u %-24d\n", reg_name, var->value.u8, var->value.u8, var->value.u8);
            break;
        case TYPE_UINT16:
            printf("  %-12s 0x%-16x %-24u %-24d\n", reg_name, var->value.u16, var->value.u16, var->value.u16);
            break;
        case TYPE_UINT32:
            printf("  %-12s 0x%-16x %-24u %-24d\n", reg_name, var->value.u32, var->value.u32, var->value.u32);
            break;
        case TYPE_UINT64:
            printf("  %-12s 0x%-16lx %-24lu %-24ld\n", reg_name, var->value.u64, var->value.u64, var->value.u64);
            break;
        case TYPE_FLOAT:
            printf("  %-12s %f\n", reg_name, var->value.flt);
            break;
        case TYPE_DOUBLE:
            printf("  %-12s %lf\n", reg_name, var->value.dbl);
            break;
        case TYPE_LONG_DOUBLE:
            printf("  %-12s %Lf\n", reg_name, var->value.l_dbl);
            break;
        case TYPE_VECTOR:
            printf("  %-12s %-36f %-36lf\n", reg_name, var->value.flt, var->value.dbl);
            break;
    }
}

void display_register(process_t* proc, const char *reg_name)
{
    int ret = 0;
    variant_t var;
    register_id id = get_register_id_by_name(reg_name);
    
    if (id == REG00_INV || id == REG64_ORIG_RAX)
    {
        printf("No register named %s\n", reg_name);
        return;
    }

    ret = get_register_by_id(proc, id, &var);
    display_variant(proc, reg_name, &var);
}

void display_gpr_registers(process_t *proc)
{
    register_id id = REG00_INV + 1;
    while (id < REG64_DR0)
    {
        if (id == REG64_ORIG_RAX)
        {
            id++;
            continue;
        }
        
        display_register(proc, get_register_name_by_id(id));
        id++;
    }
}

void display_all_registers(process_t *proc)
{
    register_id id = REG00_INV + 1;
    while (id <= REGFP_XMM15)
    {
        if (id == REG64_ORIG_RAX)
        {
            id++;
            continue;
        }
        
        display_register(proc, get_register_name_by_id(id));
        id++;
    }
}

void cli_repl()
{
    char *line;
    tokens_t tokens = {0};
    action_t action = ACTION_INVALID;
    printf("Welcome to breakpoint!\n");

    while ((line = linenoise("bkpt> ")) != NULL)
    {
        // Do nothing if line is empty
        if (line[0] == '\0')
        {
            free(line);
            continue;
        }

        action = process_line(line, &tokens);
        if (action == ACTION_QUIT)
        {
            free_tokens(&tokens);
            free(line);
            break;
        }
        else if (action == ACTION_INVALID)
        {
            printf("Invalid command.\n");
            linenoiseHistoryAdd(line);
            free(line);
            continue;
        }
        else if (action == ACTION_INCOM)
        {
            printf("Incomplete command.\n");
            linenoiseHistoryAdd(line);
            free(line);
            continue;
        }
        else if (action == ACTION_AMBIGUOUS)
        {
            printf("Ambiguous command.\n");
            linenoiseHistoryAdd(line);
            free(line);
            continue;
        }
        else if (action == ACTION_CONT)
        {
            unsigned char info;
            resume_proc(&debug_proc);
            if (wait_proc(&debug_proc, &info) == 0)
            {
                printf("%s\n", get_stop_reason(&debug_proc, info));
            }
        }
        else if (action == ACTION_READ_REG_GPR)
        {
            display_gpr_registers(&debug_proc);
        }
        else if (action == ACTION_READ_REG_ALL)
        {
            display_all_registers(&debug_proc);
        }
        else if (action == ACTION_READ_REG_ONE)
        {
            display_register(&debug_proc, tokens.list[2]);
        }
        else if (action == ACTION_WRITE_REG)
        {
            if (set_register_by_name_value(&debug_proc, 
                tokens.list[2], tokens.list[3]))
            {
                printf("%s\n", get_breakpoint_err());
            }
        }

        free_tokens(&tokens);
        linenoiseHistoryAdd(line);
        free(line);
    }
}

void print_usage(char *exe_name)
{
    printf("Usage: %s -p <pid>\n", exe_name);
    printf("Usage: %s <executable-file> [args]\n", exe_name);
}

int main(int argc, char *argv[])
{
    int ret = 0;
    pid_t debug_pid = 0;

    if (argc == 1)
    {
        print_usage(argv[0]);
        return 1;
    }

    while((ret = getopt(argc, argv,"p:h")) != -1)
    {
        switch (ret)
        {
            case 'p':
                debug_pid = atoi(optarg);
                if (debug_pid <= 0)
                {
                    fprintf(stderr, "Unable to parse process id [%s]\n", optarg);
                    return 1;
                }

                if (attach_proc(&debug_proc, debug_pid))
                    return 1;

                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    if (debug_pid == 0 && launch_proc(&debug_proc, argv + 1))
    {
        return 1;
    }

    cli_repl();
    cleanup_proc(&debug_proc);

    return 0;
}
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

process_t debug_proc = {0};

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
                print_stop_reason(&debug_proc, info);
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
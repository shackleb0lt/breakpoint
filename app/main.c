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
#include "breakpoint.h"
#include "commands.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const char *action_str[] =
{
    "ACTION_INVALID",
    "ACTION_INCOM",
    "ACTION_AMBIGUOUS",
    "ACTION_PRINTF",
    "ACTION_CONV",
    "ACTION_TODO",
    "ACTION_HELP",
    "ACTION_QUIT",
};

// Callback for tab-completion
void completion(const char *buf, linenoiseCompletions *lc)
{
    if (buf[0] == 'h')
    {
        linenoiseAddCompletion(lc, "help");
        linenoiseAddCompletion(lc, "history");
    }
    else if (buf[0] == 'e')
    {
        linenoiseAddCompletion(lc, "exit");
        linenoiseAddCompletion(lc, "echo");
    }
}

int main(int argc, char *argv[])
{

    char *line;
    tokens_t tokens = {0};
    action_t action = ACTION_INVALID;

    // Setup the UI enhancements
    linenoiseSetCompletionCallback(completion);

    printf("Welcome to the breakpoint!\n");
    printf("Type 'exit' to quit. Use Up/Down arrows for history.\n\n");

    // The Main Loop
    while ((line = linenoise("bkpt> ")) != NULL)
    {
        // Do nothing if line is empty
        if (line[0] == '\0')
        {
            free(line);
            continue;
        }
    
        // 1. Handle "exit"
        if (!strcmp(line, "exit"))
        {
            free(line);
            break;
        }
        action = process_line(line, &tokens);
        if (action != ACTION_INVALID || action != ACTION_AMBIGUOUS)
            free_tokens(&tokens);
        
        printf("%s\n", action_str[action]);

        // 3. Save to history
        linenoiseHistoryAdd(line);
        free(line);
    }

    return 0;
}
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

#ifndef BKPT_APP_COMMANDS_H
#define BKPT_APP_COMMANDS_H

typedef enum _action_t {
    ACTION_INVALID = 0,
    ACTION_INCOM,
    ACTION_AMBIGUOUS,
    ACTION_PRINTF,
    ACTION_CONV,
    ACTION_TODO,
    ACTION_HELP,
    ACTION_QUIT,
} action_t;

#define MAX_LINE_LEN 4096
#define MIN_TOKEN_COUNT 4

typedef struct _command_t
{
    const char *keyword;
    enum _action_t action;
    struct _command_t *children;
} command_t;

typedef struct _tokens_t
{
    size_t count;
    size_t capacity;
    char *copy;
    char **list;
} tokens_t;

void free_tokens(tokens_t *tokens);
action_t process_line(const char *line, tokens_t *tokens);

#endif
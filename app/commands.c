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

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "commands.h"


command_t number2[] = 
{
    {"",            ACTION_CONV,   NULL},
    {NULL,          ACTION_INVALID, NULL}
};

command_t conv_to_cmd[] = 
{
    {"hex",         ACTION_INCOM,   number2},
    {"dec",         ACTION_INCOM,   number2},
    {NULL,          ACTION_INVALID, NULL}
};

command_t number1[] = 
{
    {"",            ACTION_INCOM,   conv_to_cmd},
    {NULL,          ACTION_INVALID, NULL}
};

command_t conv_from_cmd[] = 
{
    {"hex",         ACTION_INCOM,   number1},
    {"dec",         ACTION_INCOM,   number1},
    {NULL,          ACTION_INVALID, NULL}
};

command_t top_level[] =
{
    {"convert",     ACTION_INCOM,   conv_from_cmd},
    {"help",        ACTION_HELP,    NULL},
    {"printf",      ACTION_PRINTF,  NULL},
    {"quit",        ACTION_QUIT,    NULL},
    {NULL,          ACTION_INVALID, NULL}
};

void free_tokens(tokens_t *tokens)
{
    if (tokens->copy)
    {
        free(tokens->copy);
        tokens->copy = NULL;
    }
    if (tokens->list)
    {
        free(tokens->list);
        tokens->list = NULL;
    }
    tokens->count = 0;
}

size_t tokenize_line(const char *line, tokens_t *tokens)
{
    char *token = NULL;
    char *save = NULL;

    assert(line != NULL);
    assert(tokens != NULL);
    
    tokens->count = 0;
    tokens->capacity = MIN_TOKEN_COUNT;
    tokens->list = malloc(MIN_TOKEN_COUNT * sizeof(char *));
    if (tokens->list == NULL)
    {
        return 0;
    }

    tokens->copy = strdup(line);
    if (tokens->copy == NULL)
    {
        free(tokens->list);
        tokens->list = NULL;
        return 0;
    }

    token = strtok_r(tokens->copy, " \t\r\n", &save);
    while (token != NULL)
    {
        if (tokens->count >= tokens->capacity)
        {
            tokens->capacity *= 2;
            char **temp = realloc(tokens->list, tokens->capacity * sizeof(char *));
            if (temp == NULL)
            {
                free_tokens(tokens);
                return 0;
            }
            tokens->list = temp;
        }

        tokens->list[tokens->count++] = token;
        token = strtok_r(NULL, " \t\r\n", &save);
    }

    if (tokens->count == 0)
    {
        free_tokens(tokens);
        return 0;
    }

    return tokens->count;
}

size_t compare_token(command_t *list, command_t **res, const char *token)
{
    size_t count = 0;
    command_t *curr = list;
    size_t token_len = strlen(token);

    while (curr->keyword != NULL)
    {
        // If exact match is found,
        // Return the current command entry
        if (strcmp(token, curr->keyword) == 0)
        {
            *res = curr;
            return 1;
        }
        
        // If keyword is "" then its a match by default
        // Or initial bytes of token are same
        if (curr->keyword[0] == '\0' ||
            strncmp(token, curr->keyword, token_len) == 0)
        {
            *res = curr;
            count++;
        }
        curr++;
    }
    return count;
}

action_t process_line(const char *line, tokens_t *tokens)
{
    size_t curr = 0;
    size_t matches = 0;
    action_t ret = ACTION_INVALID;

    command_t *cmds = top_level;
    command_t *res_cmd = NULL;

    if (tokenize_line(line, tokens) == 0)
        return ACTION_QUIT;

    for (curr = 0; curr < tokens->count; curr++)
    {
        if (cmds == NULL)
            break;
        matches = compare_token(cmds, &res_cmd, tokens->list[curr]);
        if (matches == 0)
        {
            free_tokens(tokens);
            return ACTION_INVALID;
        }
        else if (matches > 1)
        {
            free_tokens(tokens);
            return ACTION_AMBIGUOUS;
        }

        cmds = res_cmd->children;
    }
    
    if (curr < tokens->count)
    {
        free_tokens(tokens);
        return ACTION_INVALID;
    }

    if (res_cmd->action == ACTION_INVALID ||
        res_cmd->action == ACTION_INCOM)
    {
        free_tokens(tokens);
    }
    return res_cmd->action;
}
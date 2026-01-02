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

#ifndef BKPT_LIB_PROCESS_H
#define BKPT_LIB_PROCESS_H

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <error.h>
#include <errno.h>

typedef enum _process_state_t
{
    PROC_INIT = 0,
    PROC_STOPPED,
    PROC_RUNNING,
    PROC_EXITED,
    PROC_KILLED,
} process_state_t;

typedef struct _process_t
{
    pid_t pid;
    process_state_t state;
    bool kill_on_end;
} process_t;

int resume_proc(process_t *proc);
int launch_proc(process_t *proc, char *const argv[]);
int attach_proc(process_t *proc, pid_t debug_pid);
int wait_proc(process_t *proc, unsigned char *p_info);

void cleanup_proc(process_t *proc);
void print_stop_reason(process_t *proc, unsigned char info);

#endif

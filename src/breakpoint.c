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

#include <errno.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include "breakpoint.h"

void init_proc_struct(process_t * proc)
{
    proc->pid = 0;
    proc->state = PROC_STOPPED;
    proc->kill_on_end = false;
    proc->is_attached = false;
}

int wait_proc(process_t *proc, unsigned char *p_info)
{
    int options = 0;
    int wait_status = 0;
    unsigned char info = 0;

    if (p_info == NULL)
        p_info = &info;

    if (waitpid(proc->pid, &wait_status, options) < 0)
    {
        perror("wait_proc waitpid:");
        return -1;
    }

    if (WIFEXITED(wait_status)) {
        proc->state = PROC_EXITED;
        *p_info = WEXITSTATUS(wait_status);
    }
    else if (WIFSIGNALED(wait_status)) {
        proc->state = PROC_KILLED;
        *p_info = WTERMSIG(wait_status);
    }
    else if (WIFSTOPPED(wait_status)) {
        proc->state = PROC_STOPPED;
        *p_info = WSTOPSIG(wait_status);
    }
    
    return 0;
}

int resume_proc(process_t *proc)
{
    if (ptrace(PTRACE_CONT, proc->pid, NULL, NULL) < 0)
    {
        perror("resume_proc ptrace:");
        return -1;
    }

    proc->state = PROC_RUNNING;
    return 0;
}

int attach_proc(process_t *proc, pid_t debug_pid)
{
    int status = 0;
    if (ptrace(PTRACE_ATTACH, debug_pid, NULL, NULL) < 0)
    {
        perror("attach_proc ptrace");
        return -1;
    }

    if (waitpid(debug_pid, &status, 0) < 0)
    {
        perror("attach_proc waitpid:");
        return -1;
    }

    if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGSTOP)
    {
        proc->is_attached = true;
        proc->pid = debug_pid;
        proc->kill_on_end = false;
        proc->state = PROC_STOPPED;
        return 0;
    }

    return -1;
}

int launch_proc(process_t *proc, const char *argv[])
{
    int status = 0;
    pid_t debug_pid = 0;
    
    debug_pid = fork();
    if (debug_pid < 0)
    {
        perror("launch_proc fork:");
        return -1;
    }

    if (debug_pid == 0)
    {
        if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) < 0)
        {
            perror("launch_proc ptrace:");
            exit(1);
        }

        execvp(argv[0], (char *const *)argv);

        perror("launch_proc execv");
        exit(1);
    }

    if (waitpid(debug_pid, &status, 0) < 0)
    {
        perror("launch_proc waitpid");
        return -1;
    }

    if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP)
    {
        proc->kill_on_end = true;
        proc->pid = debug_pid;
        proc->is_attached = true;
        proc->state = PROC_STOPPED;
        return 0;
    }

    return -1;
}

void cleanup_proc(process_t *proc)
{
    int status = 0;

    if (proc->pid == 0)
        return;
    
    if (proc->is_attached)
    {
        if (proc->state == PROC_RUNNING)
        {
            kill(proc->pid, SIGSTOP);
            waitpid(proc->pid, &status, 0);
        }

        ptrace(PTRACE_DETACH, proc->pid, NULL, NULL);
        kill(proc->pid, SIGCONT);
    }

    if (proc->kill_on_end)
    {
        kill(proc->pid, SIGKILL);
        waitpid(proc->pid, &status, 0);
    }
}
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

#define _GNU_SOURCE

#include <stdio.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "breakpoint.h"

#define BUF_SIZE 256

void perror_pipe(int fd, const char *prefix)
{
    int len = 0;
    char buf[BUF_SIZE] = {0};
    int save_errno = errno;

    len = snprintf(buf, BUF_SIZE, "%s: %s\n", prefix, strerror(save_errno));
    
    if (len <= 0 || len >= BUF_SIZE)
        return;

    write(fd, buf, len);
}

void init_proc_struct(process_t * proc)
{
    proc->pid = 0;
    proc->state = PROC_INIT;
    proc->kill_on_end = false;
}

void print_stop_reason(process_t *proc, unsigned char info)
{
    printf("Process %d ", proc->pid);
    switch (proc->state)
    {
        case PROC_EXITED:
            printf("exited with status %d\n", info);
            break;
        case PROC_KILLED:
            printf("terminated with signal %s\n", strsignal(info));
            break;
        case PROC_STOPPED:
            printf("stopped with signal %s\n", strsignal(info));
            break;
    }
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
    unsigned char info = 0;
    if (ptrace(PTRACE_ATTACH, debug_pid, NULL, NULL) < 0)
    {
        perror("attach_proc ptrace");
        return -1;
    }

    proc->kill_on_end = false;
    proc->pid = debug_pid;
    if (wait_proc(proc, &info))
        return -1;

    if (proc->state == PROC_STOPPED && info == SIGSTOP)
    {
        proc->state = PROC_STOPPED;
        return 0;
    }

    // By design it should never reach here
    print_stop_reason(proc, info);
    return -1;
}

int launch_proc(process_t *proc, char *const argv[])
{
    ssize_t b_read = 0;
    pid_t debug_pid = 0;
    int pipe_fd[2] = {0};
    unsigned char info = 0;
    char buf[BUF_SIZE] = {0};

    if (pipe2(pipe_fd, O_CLOEXEC) < 0)
    {
        perror("launch_proc pipe2");
        return -1;
    }
    
    debug_pid = fork();
    if (debug_pid < 0)
    {
        perror("launch_proc fork:");
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        return -1;
    }

    if (debug_pid == 0)
    {
        close(pipe_fd[0]);

        if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) < 0)
        {
            perror_pipe(pipe_fd[1], "launch_proc ptrace");
            close(pipe_fd[1]);
            exit(1);
        }

        if (strchr(argv[0], '/') != NULL)
        {
            execvp(argv[0], (char *const *) argv);
        }
        else
        {
            snprintf(buf, BUF_SIZE, "./%s", argv[0]);
            if (access(buf, X_OK) == 0)
                execvp(buf, (char *const *)argv);

            else
                execvp(argv[0], (char *const *)argv);
        }

        snprintf(buf, BUF_SIZE, "launch_proc execvp %s", argv[0]);
        perror_pipe(pipe_fd[1], buf);
        close(pipe_fd[1]);
        exit(1);
    }

    // Must come before read, or else read will not get EOF
    close(pipe_fd[1]); 

    b_read = read(pipe_fd[0], buf, BUF_SIZE - 1);
    close(pipe_fd[0]);

    if (b_read > 0)
    {
        buf[b_read] = '\0';
        waitpid(debug_pid, NULL, 0);
        fprintf(stderr, "%s", buf);
        return -1;
    }

    proc->pid = debug_pid;
    if (wait_proc(proc, &info))
        return -1;

    if (proc->state == PROC_STOPPED && info == SIGTRAP)
    {
        proc->kill_on_end = true;
        return 0;
    }

    // By design it should never reach here
    print_stop_reason(proc, info);
    return -1;
}

void cleanup_proc(process_t *proc)
{
    int status = 0;
    if (proc->pid <= 0)
        return;
    
    if (proc->state == PROC_RUNNING)
    {
        kill(proc->pid, SIGSTOP);
        wait_proc(proc, NULL);
    }

    if (proc->state == PROC_STOPPED)
    {
        ptrace(PTRACE_DETACH, proc->pid, NULL, NULL);
        kill(proc->pid, SIGCONT);
    }

    if (proc->kill_on_end)
    {
        kill(proc->pid, SIGKILL);
        waitpid(proc->pid, &status, 0);
    }
}
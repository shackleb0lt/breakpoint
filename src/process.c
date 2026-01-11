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
#include <stdarg.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "process.h"
#include "registers.h"

#define BUF_SIZE 256

typedef const register_info_t * crit;

char g_error_string[BUF_SIZE] = {0};

const char *get_breakpoint_err()
{
    return (const char *)g_error_string;
}

void save_err(const char *fmt, ...)
{
    int len = 0;
    va_list args;
    int saved_errno = errno;
    char *ptr = g_error_string;

    va_start(args, fmt);
    len = vsnprintf(ptr, BUF_SIZE - 1, fmt, args);
    va_end(args);

    if (len < 0 || len >= BUF_SIZE - 1)
        return;
    if (saved_errno == 0)
        return;

    snprintf(ptr + len, BUF_SIZE - 1 - len, " : %s", strerror(saved_errno));
    errno = 0;
}

static int read_registers_proc(process_t *proc)
{
    int ret = 0;
    int64_t reg_data = 0;

    ret = ptrace(PTRACE_GETREGS, proc->pid, NULL, &(proc->data.regs));
    if (ret < 0)
    {
        save_err("%s ptrace(PTRACE_GETREGS)", __func__);
        return -1;
    }

    ret = ptrace(PTRACE_GETFPREGS, proc->pid, NULL, &(proc->data.i387));
    if (ret < 0)
    {
        save_err("%s ptrace(PTRACE_GETFPREGS)", __func__);
        return -1;
    }

    for (size_t i = 0; i < 8; i++)
    {
        crit reg_info = get_register_info_by_id(REG64_DR0 + i);
        if (reg_info == NULL)
        {
            save_err("%s get_register_info_by_id(%lu) returned NULL", __func__, REG64_DR0 + i);
            return -1;
        }

        errno = 0;
        reg_data = ptrace(PTRACE_PEEKUSER, proc->pid, reg_info->offset, NULL);
        if (errno != 0)
        {
            save_err("%s ptrace(PTRACE_PEEKUSER)", __func__);
            return -1;
        }

        proc->data.u_debugreg[i] = reg_data;
    }

    return 0;
}

static int write_gprs_proc(process_t *proc, struct user_regs_struct *gprs)
{
    int ret = ptrace(PTRACE_SETREGS, proc->pid, NULL, gprs);
    if (ret < 0)
    {
        save_err("%s ptrace(PTRACE_SETREGS)", __func__);
        return -1;
    }

    return 0;
}

static int write_fprs_proc(process_t *proc, struct user_fpregs_struct *fprs)
{
    int ret = ptrace(PTRACE_SETFPREGS, proc->pid, NULL, fprs);
    if (ret < 0)
    {
        save_err("%s ptrace(PTRACE_SETFPREGS)", __func__);
        return -1;
    }

    return 0;
}

static int poke_register_proc(process_t *proc, size_t off, uint64_t data)
{
    int ret = 0;
    ret = ptrace(PTRACE_POKEUSER, proc->pid, off, data);
    if (ret < 0)
    {
        save_err("%s ptrace(PTRACE_POKEUSER)", __func__);
        return -1;
    }

    return 0;
}

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

const char *get_stop_reason(process_t *proc, unsigned char info)
{
    static char stop_str[BUF_SIZE] = {0};
    char *ptr = stop_str; 
    int len = 0;

    len = snprintf(ptr, BUF_SIZE - 1, "Process %d ", proc->pid);
    ptr += len;

    switch (proc->state)
    {
        case PROC_EXITED:
            snprintf(ptr, BUF_SIZE - 1 - len, "exited with status %d\n", info);
            break;
        case PROC_KILLED:
            snprintf(ptr, BUF_SIZE - 1 - len, "terminated with signal %s\n", strsignal(info));
            break;
        case PROC_STOPPED:
            snprintf(ptr, BUF_SIZE - 1 - len, "stopped with signal %s\n", strsignal(info));
            break;
    }

    return (const char *)stop_str;
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
        save_err("%s waitpid", __func__);
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
        return read_registers_proc(proc);
    }

    return 0;
}

int resume_proc(process_t *proc)
{
    if (ptrace(PTRACE_CONT, proc->pid, NULL, NULL) < 0)
    {
        save_err("%s ptrace(PTRACE_CONT)", __func__);
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
        save_err("%s ptrace(PTRACE_ATTACH)", __func__);
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
    save_err("Attach failed as %s", get_stop_reason(proc, info));
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
        save_err("%s pipe2", __func__);
        return -1;
    }
    
    debug_pid = fork();
    if (debug_pid < 0)
    {
        save_err("%s fork", __func__);
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        return -1;
    }

    if (debug_pid == 0)
    {
        close(pipe_fd[0]);

        if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) < 0)
        {
            perror_pipe(pipe_fd[1], "launch_proc ptrace(PTRACE_TRACEME)");
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
        save_err("%s", buf);
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
    save_err("Launch failed as %s", get_stop_reason(proc, info));
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
        proc->state = PROC_RUNNING;
    }

    if (proc->kill_on_end)
    {
        kill(proc->pid, SIGKILL);
        waitpid(proc->pid, &status, 0);
        proc->state = PROC_KILLED;
    }

    errno = 0;
}

int get_register_by_id(process_t *proc, register_id id, void *buf, size_t buf_size)
{
    uint8_t *dest_bytes = (uint8_t *)buf;
    uint8_t *src_bytes = (uint8_t *)&(proc->data);
    crit reg_info = get_register_info_by_id(id);

    if (reg_info == NULL)
    {
        save_err("%s no register info of ID %lu", id);
        return -1;
    }

    if (buf_size > reg_info->size)
    {
        save_err("%s register %lu cannot be larger than %lu bytes",
            __func__, id, reg_info->size);
        return -1;
    }

    memset(dest_bytes, 0, buf_size);
    memcpy(dest_bytes, src_bytes + reg_info->offset, buf_size);
    return 0;
}

int set_register_by_id(process_t *proc, register_id id, void *buf, size_t buf_size)
{
    uint64_t data = 0;
    size_t aligned_offset = 0;

    uint8_t *src_bytes = (uint8_t *)buf;
    uint8_t *dest_bytes = (uint8_t *)&(proc->data);
    crit reg_info = get_register_info_by_id(id);

    if (reg_info == NULL)
    {
        save_err("%s no register info of ID %lu", id);
        return -1;
    }

    if (buf_size > reg_info->size)
    {
        save_err("%s register %lu cannot be larger than %lu bytes",
            __func__, id, reg_info->size);
        return -1;
    }

    memcpy(dest_bytes + reg_info->offset, src_bytes, buf_size);

    if (reg_info->id >= REGFP_CWD)
    {
        return write_fprs_proc(proc, &(proc->data.i387));
    }

    aligned_offset = reg_info->offset & (~0b111);
    data = *((uint64_t *)(dest_bytes + aligned_offset));
    return poke_register_proc(proc, aligned_offset, data);
}

int get_register_by_name(process_t *proc, char *name, void *buf, size_t buf_size)
{
    uint8_t *dest_bytes = (uint8_t *)buf;
    uint8_t *src_bytes = (uint8_t *)&(proc->data);
    crit reg_info = get_register_info_by_name(name);

    if (reg_info == NULL)
    {
        save_err("%s no register by name %s", name);
        return -1;
    }

    if (buf_size > reg_info->size)
    {
        save_err("%s register %s cannot be larger than %lu bytes",
            __func__, name, reg_info->size);
        return -1;
    }

    memset(dest_bytes, 0, buf_size);
    memcpy(dest_bytes, src_bytes + reg_info->offset, buf_size);
    return 0;
}

int set_register_by_name(process_t *proc, char *name, void *buf, size_t buf_size)
{
    uint64_t data = 0;
    size_t aligned_offset = 0;

    uint8_t *src_bytes = (uint8_t *)buf;
    uint8_t *dest_bytes = (uint8_t *)&(proc->data);
    crit reg_info = get_register_info_by_name(name);

    if (reg_info == NULL)
    {
        save_err("%s no register by name %s", name);
        return -1;
    }

    if (buf_size > reg_info->size)
    {
        save_err("%s register %s cannot be larger than %lu bytes",
            __func__, name, reg_info->size);
        return -1;
    }

    memcpy(dest_bytes + reg_info->offset, src_bytes, buf_size);

    if (reg_info->id >= REGFP_CWD)
    {
        return write_fprs_proc(proc, &(proc->data.i387));
    }

    aligned_offset = reg_info->offset & (~0b111);
    data = *((uint64_t *)(dest_bytes + aligned_offset));
    return poke_register_proc(proc, aligned_offset, data);
}
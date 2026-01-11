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

#include "process.h"

#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/wait.h>

#define BUF_SIZE 256

bool process_exists(pid_t pid)
{
    int ret = 0;
    if (pid <= 0)
        return false;

    ret = kill(pid, 0);
    if (ret == 0)
        return true;
    
    if (errno == EPERM)
        return true;
    
    return false;
}

/*
|----------------------------------------------------------------------------|
| State | Name         | Meaning                                             |
| ----- | ------------ | --------------------------------------------------- |
|   R   | Running      | Currently running **or runnable** (on run queue)    |
|   S   | Sleeping     | Interruptible sleep (waiting for event, signalable) |
|   D   | Disk sleep   | Uninterruptible sleep (usually I/O)                 |
|   T   | Stopped      | Stopped by job control (`SIGSTOP`, `SIGTSTP`)       |
|   t   | Tracing stop | Stopped by debugger (`ptrace`)                      |
|   Z   | Zombie       | Exited, parent hasn’t `wait()`ed                    |
|   X   | Dead         | Being removed (rarely seen)                         |
|   I   | Idle         | Idle kernel thread                                  |
|----------------------------------------------------------------------------|
*/
char process_status(pid_t pid)
{
    FILE *stat_file = NULL;
    char buf[1024] = {0};
    char *ptr = NULL;

    if (pid <= 0)
        return 'N';

    snprintf(buf, 1024, "/proc/%d/stat", pid);

    stat_file = fopen(buf, "r");
    if (stat_file == NULL)
    {
        perror("fopen");
        return 'N';
    }

    fread(buf, 1024, sizeof(char), stat_file);
    fclose(stat_file);

    ptr = strchr(buf, ')');
    if (ptr == NULL)
    {
        return 'N';
    }

    return ptr[2];
}

bool process_running(pid_t pid)
{
    char st = process_status(pid);
    if ((st == 'R') || (st == 'S') || (st == 'D'))
        return true;
    return false;
}

pid_t process_create(char *const argv[])
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
        perror("process_create fork:");
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        return -1;
    }

    if (debug_pid == 0)
    {
        close(pipe_fd[0]);
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

        snprintf(buf, BUF_SIZE, "process_create execvp %s %s", argv[0], strerror(errno));
        write(pipe_fd[1], buf, strlen(buf));
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

    return debug_pid;
}

void process_destroy(pid_t pid)
{
    int status;
    kill(pid, SIGKILL);
    waitpid(pid, &status, 0);
}
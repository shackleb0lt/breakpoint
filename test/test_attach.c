#define _GNU_SOURCE

#include "test_common.h"
#include <sys/wait.h>

pid_t create_proc(char *const argv[])
{
    ssize_t b_read = 0;
    pid_t debug_pid = 0;
    int pipe_fd[2] = {0};
    unsigned char info = 0;
    char buf[BUF_SIZE] = {0};

    if (pipe2(pipe_fd, O_CLOEXEC) < 0)
    {
        perror("create_proc pipe2");
        return -1;
    }
    
    debug_pid = fork();
    if (debug_pid < 0)
    {
        perror("create_proc fork:");
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

        snprintf(buf, BUF_SIZE, "create_proc execvp %s %s", argv[0], strerror(errno));
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

int main()
{
    char status = 0;
    pid_t cpid = 0;
    unsigned char info = 0;
    process_t debug_proc = {0};

    char *const exec[] =
    {
        "./debug_bin",
        "3",
        // "/tmp/dummy_attach.bin",
        NULL
    };

    // Test Non Existent Process
    if (process_exists(999999) == false)
        assert(attach_proc(&debug_proc, 999999) == -1);

    // Test insufficient permission    
    if (process_exists(1) == true && errno == EPERM)
        assert(attach_proc(&debug_proc, 1) == -1);

    // Launch genuine process
    cpid = create_proc(exec);
    assert(process_exists(cpid) == true);

    // Attach to genuine process
    assert(attach_proc(&debug_proc, cpid) == 0);
    assert(get_process_status(debug_proc.pid) == 't');
    assert(debug_proc.state == PROC_STOPPED);

    // Resume the process
    assert(resume_proc(&debug_proc) == 0);
    status = get_process_status(debug_proc.pid);
    assert((status == 'R') || (status == 'S'));
    assert(debug_proc.state == PROC_RUNNING);

    // Wait for process to finish
    assert(wait_proc(&debug_proc, &info) == 0);
    assert(debug_proc.state == PROC_EXITED);

    debug_proc.kill_on_end = true;
    cleanup_proc(&debug_proc);

    assert(process_exists(debug_proc.pid) == false);

    return 0;
}
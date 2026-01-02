#include "process.h"

#include <fcntl.h>
#include <stdio.h>
#include <assert.h>

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

char get_process_status(pid_t pid)
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
#include "test_common.h"

int main()
{
    char status = 0;
    process_t debug_proc = {0};
    char *const quick[] =
    {
        "debug_bin",
        "0",
        // "/tmp/dummy_launch.bin",
        NULL
    };

    char *const normal[] =
    {
        "debug_bin",
        "3",
        NULL
    };


    char *const fake[] = 
    {
        "voldemort",
        NULL
    };

    // Test launch on a fake binary/executable
    assert(launch_proc(&debug_proc, fake) == -1);

    // Launch normal process
    assert(launch_proc(&debug_proc, normal) == 0);
    assert(process_exists(debug_proc.pid));
    assert(get_process_status(debug_proc.pid) == 't');

    assert(debug_proc.kill_on_end == true);
    assert(debug_proc.state == PROC_STOPPED);

    // Resume process execution
    assert(resume_proc(&debug_proc) == 0);
    status = get_process_status(debug_proc.pid);
    assert((status == 'R') || (status == 'S'));
    assert(debug_proc.state == PROC_RUNNING);

    // Wait for process to finish executing
    assert(wait_proc(&debug_proc, NULL) == 0);
    assert(debug_proc.state == PROC_EXITED);

    // Test resume fails after process has exited
    assert(wait_proc(&debug_proc, NULL) == -1);
    
    cleanup_proc(&debug_proc);
    assert(process_exists(debug_proc.pid) == false);

    return 0;
}
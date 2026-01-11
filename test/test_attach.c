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

#include "test_common.h"

int main()
{
    process_t debug_proc = {0};

    {
        // Test Attach to non existent process
        if (process_exists(999999) == false)
            assert(attach_proc(&debug_proc, 999999) == -1);
    }

    {
        // Test insufficient permission    
        if (process_exists(1) == true && errno == EPERM)
            assert(attach_proc(&debug_proc, 1) == -1);
    }

    {
        pid_t debug_pid = 0;
        char *const exec[] =
        {
            "two_seconds",
            NULL
        };

        // Launch genuine process
        debug_pid = process_create(exec);
        assert(process_exists(debug_pid) == true);
        assert(process_running(debug_pid) == true);

        // Attach to this genuine process
        assert(attach_proc(&debug_proc, debug_pid) == 0);
        assert(process_status(debug_proc.pid) == 't');
        assert(debug_proc.state == PROC_STOPPED);

        // Resume the process
        assert(resume_proc(&debug_proc) == 0);
        assert(process_running(debug_pid) == true);
        assert(debug_proc.state == PROC_RUNNING);

        // Wait for process to finish
        assert(wait_proc(&debug_proc, NULL) == 0);
        assert(process_exists(debug_proc.pid) == false);
        assert(debug_proc.state == PROC_EXITED);
    }

    {
        pid_t debug_pid = 0;
        char *const exec[] =
        {
            "two_seconds",
            NULL
        };

        // Launch genuine process
        debug_pid = process_create(exec);
        assert(process_exists(debug_pid) == true);
        assert(process_running(debug_pid) == true);

        // Attach to this genuine process
        assert(attach_proc(&debug_proc, debug_pid) == 0);
        assert(process_status(debug_proc.pid) == 't');
        assert(debug_proc.state == PROC_STOPPED); 

        // Resume the process
        assert(resume_proc(&debug_proc) == 0);
        assert(process_running(debug_pid) == true);
        assert(debug_proc.state == PROC_RUNNING);

        // Test detach from running process
        cleanup_proc(&debug_proc);
        assert(process_exists(debug_proc.pid) == true);
        assert(process_running(debug_pid) == true);
        assert(debug_proc.state == PROC_RUNNING);

        // Kill the process
        process_destroy(debug_pid);
    }

    {
        pid_t debug_pid = 0;
        char *const exec[] =
        {
            "two_seconds",
            NULL
        };

        // Launch genuine process
        debug_pid = process_create(exec);
        assert(process_exists(debug_pid) == true);
        assert(process_running(debug_pid) == true);

        // Attach to this genuine process
        assert(attach_proc(&debug_proc, debug_pid) == 0);
        assert(process_status(debug_proc.pid) == 't');
        assert(debug_proc.state == PROC_STOPPED); 

        // Test detach from stopped process
        cleanup_proc(&debug_proc);
        assert(process_exists(debug_proc.pid) == true);
        assert(process_running(debug_pid) == true);
        assert(debug_proc.state == PROC_RUNNING);

        // Kill the process
        process_destroy(debug_pid);
    }

    return 0;
}
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
        char *const fake[] = 
        {
            "voldemort",
            NULL
        };

        // Test launch on a fake binary/executable
        assert(launch_proc(&debug_proc, fake) == -1);
    }

    {
        char *const exec[] = 
        {
            "two_seconds",
            NULL
        };

        // Launch normal process
        assert(launch_proc(&debug_proc, exec) == 0);
        assert(process_exists(debug_proc.pid));
        assert(process_status(debug_proc.pid) == 't');
        assert(debug_proc.kill_on_end == true);
        assert(debug_proc.state == PROC_STOPPED);

        // Resume process execution
        assert(resume_proc(&debug_proc) == 0);
        assert(process_running(debug_proc.pid) == true);
        assert(debug_proc.state == PROC_RUNNING);

        // Wait for process to finish executing
        assert(wait_proc(&debug_proc, NULL) == 0);
        assert(debug_proc.state == PROC_EXITED);

        cleanup_proc(&debug_proc);
        assert(process_exists(debug_proc.pid) == false);

    }

    {
        char *const exec[] = 
        {
            "two_seconds",
            NULL
        };

        // Launch normal process
        assert(launch_proc(&debug_proc, exec) == 0);
        assert(process_exists(debug_proc.pid));
        assert(process_status(debug_proc.pid) == 't');
        assert(debug_proc.kill_on_end == true);
        assert(debug_proc.state == PROC_STOPPED);

        // Resume process execution
        assert(resume_proc(&debug_proc) == 0);
        assert(process_running(debug_proc.pid) == true);
        assert(debug_proc.state == PROC_RUNNING);

        // Kill process while running
        cleanup_proc(&debug_proc);
        assert(process_exists(debug_proc.pid) == false);

    }

    {
        char *const exec[] = 
        {
            "two_seconds",
            NULL
        };

        // Launch normal process
        assert(launch_proc(&debug_proc, exec) == 0);
        assert(process_exists(debug_proc.pid));
        assert(process_status(debug_proc.pid) == 't');
        assert(debug_proc.kill_on_end == true);
        assert(debug_proc.state == PROC_STOPPED);

        // Kill process while stopped
        cleanup_proc(&debug_proc);
        assert(process_exists(debug_proc.pid) == false);

    }

    {
        char *const exec[] = 
        {
            "outta_here",
            NULL
        };

        // Launch normal process but fast exiting process
        assert(launch_proc(&debug_proc, exec) == 0);
        assert(process_exists(debug_proc.pid));
        assert(process_status(debug_proc.pid) == 't');
        assert(debug_proc.kill_on_end == true);
        assert(debug_proc.state == PROC_STOPPED);

        // Resume process execution
        assert(resume_proc(&debug_proc) == 0);
        assert(process_running(debug_proc.pid) == true);
        assert(debug_proc.state == PROC_RUNNING);

        // Wait for process to finish executing
        assert(wait_proc(&debug_proc, NULL) == 0);
        assert(debug_proc.state == PROC_EXITED);

        cleanup_proc(&debug_proc);
        assert(process_exists(debug_proc.pid) == false);
    }

    return 0;
}
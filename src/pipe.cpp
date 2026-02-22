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

#include "pipe.hpp"
#include "error.hpp"

#include <unistd.h>
#include <fcntl.h>

Pipe::Pipe(bool close_on_exec)
{
    int ret = 0;
    if (close_on_exec)
        ret = pipe2(fds_, O_CLOEXEC);
    else
        ret = pipe2(fds_, 0);

    if (ret < 0)
    {
        Error::send_errno("Pipe creation failed");
    }
}

Pipe::~Pipe()
{
    close_read();
    close_write();
}

Pipe::Pipe(Pipe &&other) noexcept
{
    fds_[read_fd] = other.fds_[read_fd];
    fds_[write_fd] = other.fds_[write_fd];

    other.fds_[read_fd] = -1;
    other.fds_[write_fd] = -1;
}

Pipe &Pipe::operator=(Pipe &&other) noexcept
{
    if (this != &other)
    {
        close_read();
        close_write();

        // Steal from other
        fds_[read_fd] = other.fds_[read_fd];
        fds_[write_fd] = other.fds_[write_fd];

        // Invalidate other
        other.fds_[read_fd] = -1;
        other.fds_[write_fd] = -1;
    }
    return *this;
}

void Pipe::close_read()
{
    if (fds_[read_fd] != -1)
    {
        close(fds_[read_fd]);
        fds_[read_fd] = -1;
    }
}

void Pipe::close_write()
{
    if (fds_[write_fd] != -1)
    {
        close(fds_[write_fd]);
        fds_[write_fd] = -1;
    }
}

void Pipe::read(std::string &buf)
{
    buf.resize(BUF_SIZE);

    ssize_t bytes_read = ::read(fds_[read_fd], buf.data(), BUF_SIZE);
    if (bytes_read < 0)
    {
        Error::send_errno("Pipe read failed");
    }

    buf.resize(static_cast<std::size_t>(bytes_read));
}

void Pipe::write(std::string_view buf)
{
    const char *data_ptr = buf.data();
    std::size_t remaining = buf.size();

    while (remaining > 0)
    {
        ssize_t ret = ::write(fds_[write_fd], data_ptr, remaining);

        if (ret < 0)
        {
            if (errno == EINTR)
                continue;

            Error::send_errno("Pipe write failed");
        }

        data_ptr += ret;
        remaining -= ret;
    }
}

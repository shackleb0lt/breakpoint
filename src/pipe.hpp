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

#ifndef BKPT_LIB_PIPE_HPP
#define BKPT_LIB_PIPE_HPP

#include <string>
#include <string_view>
#include <vector>

class Pipe
{
public:
    explicit Pipe(bool close_on_exec);
    ~Pipe();

    Pipe(const Pipe &) = delete;
    Pipe &operator=(const Pipe &) = delete;

    Pipe(Pipe &&other) noexcept;
    Pipe &operator=(Pipe &&other) noexcept;

    // int get_read() const { return fds_[read_fd]; }
    // int get_write() const { return fds_[write_fd]; }
    // int release_read();
    // int release_write();

    void close_read();
    void close_write();

    void read(std::string &buf);
    void write(std::string_view buf);

private:
    static constexpr unsigned BUF_SIZE = 256;
    static constexpr unsigned read_fd = 0;
    static constexpr unsigned write_fd = 1;
    int fds_[2] = {-1, -1};
};

class SocketPair
{
public:
    explicit SocketPair();
    ~SocketPair();

    SocketPair(const SocketPair &) = delete;
    SocketPair &operator=(const SocketPair &) = delete;

    SocketPair(SocketPair &&other) noexcept;
    SocketPair &operator=(SocketPair &&other) noexcept;

    int release_parent();
    void dup_std_fds();

    void close_child();
    void close_parent();

private:
    static constexpr unsigned child_sock = 0;
    static constexpr unsigned parent_sock = 1;
    int sv_[2] = {-1, -1};
};

#endif
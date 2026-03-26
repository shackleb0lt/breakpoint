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

#ifndef BKPT_LIB_TYPES_H
#define BKPT_LIB_TYPES_H

#include <cstdint>
#include <vector>

using virt_addr = std::uint64_t;

// Need to write custom span class since C++17
// Analohous to string view but for any type
template <class T>
class Span
{
public:
    Span() = default;
    Span(T *data, std::size_t size) : data_(data), size_(size) {}
    Span(T *data, T *end) : data_(data), size_(end - data) {}

    template <class U>
    Span(const std::vector<U> &vec) : data_(vec.data()), size_(vec.size()) {}
    T *begin() const { return data_; }
    T *end() const { return data_ + size_; }
    std::size_t size() const { return size_; }
    T &operator[](std::size_t n) { return *(data_ + n); }

private:
    T *data_ = nullptr;
    std::size_t size_ = 0;
};

#endif
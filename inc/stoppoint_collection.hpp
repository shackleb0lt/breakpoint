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

#ifndef BKPT_LIB_STOPPOINT_H
#define BKPT_LIB_STOPPOINT_H

#include <memory>
#include <algorithm>

#include "error.hpp"
#include "types.hpp"

template <class Stoppoint>
class StoppointCollection
{
public:
    Stoppoint& push(std::unique_ptr<Stoppoint> sp);

    bool contains_id(typename Stoppoint::id_type id) const;
    bool contains_address(virt_addr address) const;
    bool enabled_stoppoint_at_address(virt_addr address) const;

    Stoppoint& get_by_id(typename Stoppoint::id_type id);
    const Stoppoint& get_by_id(typename Stoppoint::id_type id) const;
    Stoppoint& get_by_address(virt_addr address);
    const Stoppoint& get_by_address(virt_addr address) const;

    void remove_by_id(typename Stoppoint::id_type id);
    void remove_by_address(virt_addr address);

    template <typename F>
    void for_each(F f);
    template <typename F>
    void for_each(F f) const;

    std::size_t size() const { return stoppoints_.size(); }
    bool empty() const { return stoppoints_.empty(); }

    std::vector<Stoppoint *>
    get_in_region(virt_addr low, virt_addr high) const;

private:
    using points_t = std::vector<std::unique_ptr<Stoppoint>>;

    typename points_t::iterator find_by_id(typename Stoppoint::id_type id);
    typename points_t::const_iterator find_by_id(typename Stoppoint::id_type id) const;
    typename points_t::iterator find_by_address(virt_addr address);
    typename points_t::const_iterator find_by_address(virt_addr address) const;

    points_t stoppoints_;
};

template <class Stoppoint>
Stoppoint& StoppointCollection<Stoppoint>::push(std::unique_ptr<Stoppoint> sp)
{
    stoppoints_.push_back(std::move(sp));
    return *stoppoints_.back();
}

template <class Stoppoint>
typename StoppointCollection<Stoppoint>::points_t::iterator
StoppointCollection<Stoppoint>::find_by_id(typename Stoppoint::id_type id)
{
    return std::find_if(stoppoints_.begin(), stoppoints_.end(),
                        [id](const std::unique_ptr<Stoppoint> &sp)
                        { return sp->id() == id; });
}

template <class Stoppoint>
typename StoppointCollection<Stoppoint>::points_t::const_iterator
StoppointCollection<Stoppoint>::find_by_id(typename Stoppoint::id_type id) const
{
    return const_cast<StoppointCollection*>(this)->find_by_id(id);
}

template <class Stoppoint>
typename StoppointCollection<Stoppoint>::points_t::iterator
StoppointCollection<Stoppoint>::find_by_address(virt_addr address)
{
    return std::find_if(stoppoints_.begin(), stoppoints_.end(),
                        [address](const std::unique_ptr<Stoppoint> &sp)
                        { return sp->at_address(address); });
}

template <class Stoppoint>
typename StoppointCollection<Stoppoint>::points_t::const_iterator
StoppointCollection<Stoppoint>::find_by_address(virt_addr address) const
{
    return const_cast<StoppointCollection*>(this)->find_by_address(address);
}

template <class Stoppoint>
bool StoppointCollection<Stoppoint>::contains_id(typename Stoppoint::id_type id) const
{
    return find_by_id(id) != end(stoppoints_);
}

template <class Stoppoint>
bool StoppointCollection<Stoppoint>::contains_address(virt_addr address) const
{
    return find_by_address(address) != end(stoppoints_);
}

template <class Stoppoint>
bool StoppointCollection<Stoppoint>::enabled_stoppoint_at_address(virt_addr address) const
{
    return contains_address(address) && get_by_address(address).is_enabled();
}

template <class Stoppoint>
Stoppoint &
StoppointCollection<Stoppoint>::get_by_id(typename Stoppoint::id_type id)
{
    auto it = find_by_id(id);
    if (it == end(stoppoints_))
        Error::send("Invalid stoppoint id");

    return **it;
}

template <class Stoppoint>
const Stoppoint &
StoppointCollection<Stoppoint>::get_by_id(typename Stoppoint::id_type id) const
{
    return const_cast<StoppointCollection *>(this)->get_by_id(id);
}

template <class Stoppoint>
Stoppoint &
StoppointCollection<Stoppoint>::get_by_address(virt_addr address)
{
    auto it = find_by_address(address);
    if (it == end(stoppoints_))
        Error::send("Stoppoint with given address not found");

    return **it;
}

template <class Stoppoint>
const Stoppoint &
StoppointCollection<Stoppoint>::get_by_address(virt_addr address) const
{
    return const_cast<StoppointCollection *>(this)->get_by_address(address);
}

template <class Stoppoint>
void  StoppointCollection<Stoppoint>::remove_by_id(typename Stoppoint::id_type id)
{
    auto it = find_by_id(id);
    if (it == end(stoppoints_))
        return;
        // Error::send("Invalid stoppoint id");

    (**it).disable();
    stoppoints_.erase(it);
}

template <class Stoppoint>
void  StoppointCollection<Stoppoint>::remove_by_address(virt_addr addr)
{
    auto it = find_by_address(addr);
    if (it == end(stoppoints_))
        return;
        // Error::send("Stoppoint with given address not found");

    (**it).disable();
    stoppoints_.erase(it);
}

template <class Stoppoint>
template <typename F>
void StoppointCollection<Stoppoint>::for_each(F f)
{
    for (auto &point : stoppoints_)
    {
        f(*point);
    }
}

template <class Stoppoint>
template <typename F>
void StoppointCollection<Stoppoint>::for_each(F f) const
{
    for (const auto &point : stoppoints_)
    {
        f(*point);
    }
}

template <class Stoppoint>
std::vector<Stoppoint *>
StoppointCollection<Stoppoint>::get_in_region(virt_addr low, virt_addr high) const
{
    std::vector<Stoppoint *> ret;
    for (auto &site : stoppoints_)
    {
        if (site->in_range(low, high))
        {
            ret.push_back(&*site);
        }
    }
    return ret;
}

#endif
#pragma once
#include <atomic>
#include <vector>
#include <iostream>
#include <thread>
#include <chrono>

template <typename T>
class ring_buffer_old
{
public:
    ring_buffer_old(size_t capacity) :
        storage(capacity + 1),
        tail(0),
        head(0)
    {}

    bool push(T value)
    {
        size_t curr_tail = tail.load();
        size_t curr_head = head.load();

        if (get_next(curr_tail) == curr_head)
            return false;

        storage[curr_tail] = std::move(value);
        tail.store(get_next(curr_tail));

        return true;
    }

    bool pop(T& value)
    {
        size_t curr_head = head.load();
        size_t curr_tail = tail.load();

        if (curr_head == curr_tail)
            return false;

        value = std::move(storage[curr_head]);
        head.store(get_next(curr_head));

        return true;
    }

private:
    size_t get_next(size_t slot) const
    {
        return (slot + 1) % storage.size();
    }

private:
    std::vector<T> storage;
    std::atomic<size_t> tail;
    std::atomic<size_t> head;
};

//template <typename T>
//class ring_buffer_old
//{
//public:
//	ring_buffer_old(size_t capacity);
//
//	bool push(T value);
//	bool pop(T &value);
//private:
//	size_t get_next(size_t slot) const;
//
//private:
//	std::vector<T> storage;
//	std::atomic<size_t> tail;
//	std::atomic<size_t> head;
//};

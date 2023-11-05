#include <atomic>
#include <vector>
#include <iostream>

/*

1. Использование std::memory_order_acquire, std::memory_order_release,
и std::memory_order_relaxed помогает уточнить ограничения видимости
операций чтения и записи для потоков.

2. Использование кэша. Когда head впервые обнаруживает, что доступно
N элементов для чтения, он кэширует эту информацию, и последующие
N-1 операций чтения не потребуют чтения индекса записи. Аналогично,
когда tail впервые обнаруживает, что доступно N элементов для записи,
он кэширует эту информацию, и последующие N-1 операций записи не потребуют
чтения индекса чтения.

3. Выравнивание полей под строки кэша (alignas).
Для процессоров ARM, AMDx64 - значение 64 байта.
*/

template <typename T>
class ring_buffer
{
public:
    ring_buffer(size_t capacity) :
        m_storage(capacity + 1, 0),
        m_capacity(capacity + 1),
        m_tail(0),
        m_head(0),
        m_tail_cached(0),
        m_head_cached(0)
    {}

    bool push(T value)
    {
        const size_t curr_tail = m_tail.load(std::memory_order_relaxed);


        size_t next_tail = curr_tail + 1;
        if (next_tail == m_capacity)
            next_tail = 0;

        if (next_tail == m_head_cached)
        {
            m_head_cached = m_head.load(std::memory_order_acquire);
			if (next_tail == m_head_cached)
				return false;
        }

        m_storage[curr_tail] = std::move(value);
        m_tail.store(next_tail, std::memory_order_release);

        return true;
    }

    bool pop(T& value)
    {
        size_t curr_head = m_head.load(std::memory_order_relaxed);

        if (curr_head == m_tail_cached)
        {
            m_tail_cached = m_tail.load(std::memory_order_acquire);
            if (curr_head == m_tail_cached)
                return false;
        }

        value = std::move(m_storage[curr_head]);
        size_t next = curr_head + 1;
        if (next == m_capacity)
            next = 0;
        m_head.store(next, std::memory_order_release);

        return true;
    }

private:
    size_t get_next(size_t slot) const
    {
        return (slot + 1) % m_capacity;
    }

private:
    std::vector<T> m_storage;
    alignas(64) std::atomic<size_t> m_capacity;
    alignas(64) std::atomic<size_t> m_tail;
    alignas(64) std::atomic<size_t> m_head;
    alignas(64) size_t m_tail_cached;
    alignas(64) size_t m_head_cached;
};
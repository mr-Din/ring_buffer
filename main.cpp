#include <iostream>
#include <Windows.h>
#include "ring_buffer_old.h"
#include "ring_buffer.h"

template<typename T>
size_t test(T& buffer, const std::string& name)
{
    int count = 10000000;

    //ring_buffer_old<int> buffer(1024);

    auto start = std::chrono::steady_clock::now();

    std::thread producer([&]() {
        for (int i = 0; i < count; ++i)
        {
            while (!buffer.push(i))
                std::this_thread::yield();
        }
        });

    uint64_t sum = 0;

    std::thread consumer([&]() {
        for (int i = 0; i < count; ++i)
        {
            int value;

            while (!buffer.pop(value))
                std::this_thread::yield();

            sum += value;
        }
        });

    producer.join();
    consumer.join();

    auto finish = std::chrono::steady_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();

    std::cout << name << ":\t time: " << ms << "ms ";
    std::cout << "sum: " << sum << std::endl;
    return ms;
}

int main()
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    ring_buffer_old<int> buffer_old(1024);
    ring_buffer<int> buffer_my(1024);
    size_t sum1 = 0;
    size_t sum2 = 0;
    int count = 20;
    for (int i = 0; i < count; ++i)
    {
        sum1 += test(buffer_old, "old");
        sum2 += test(buffer_my, "my");
    }
    size_t av_old = sum1 / count;
    size_t av_my = sum2 / count;
    std::cout << "Время исходного класса: " << av_old << std::endl;
    std::cout << "Время нового класса: " << av_my << std::endl;

    std::cout << "Класс ring_buffer эффективнее ring_buffer_old на "
        << (int)(100 * (1 - (double)av_my / (double)av_old)) << " %";

    return 0;
}

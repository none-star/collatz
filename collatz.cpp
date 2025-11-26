#include "collatz.h"
#include <thread>
#include <vector>
#include <atomic>
#include <limits>
#include <chrono>


int count_zeros(uint64_t n)
{
    if (n == 0)
    {
        return 64;
    }
    int zeros = 0;
    while ((n & 1) == 0)
    {
        n >>= 1;
        zeros++;
    }
    return zeros;
}


const uint64_t SAFE_LIMIT = (std::numeric_limits<uint64_t>::max() - 1) / 3;

CollatzSolver::CollatzSolver() : stop_flag(false) {}
CollatzSolver::~CollatzSolver() = default;


void CollatzSolver::request_stop()
{
    stop_flag.store(true, std::memory_order_relaxed);
}


uint64_t calc_length(uint64_t n, bool& out_overflow)
{
    uint64_t length = 1;

    while (n != 1)
    {
        if ((n & 1) == 0) {
            int zeros = count_zeros(n);
            n >>= zeros;
            length += zeros;
        } else {
            if (n > SAFE_LIMIT) {
                out_overflow = true;
                return 0;
            }
            n = (n * 3 + 1) >> 1;
            length += 2;
        }
    }
    return length;
}

void CollatzSolver::compute(uint64_t limit, unsigned num_threads)
{
    best_start_ = 0;
    best_length_ = 0;
    stopped_ = false;
    overflow_ = false;
    elapsed_ms_ = 0;
    message_.clear();
    stop_flag.store(false);

    if (num_threads < 1)
    {
        num_threads = 1;
    }

    std::atomic<bool> global_overflow(false);

    struct ThreadResult
    {
        uint64_t max_len = 0;
        uint64_t max_val = 0;
    };

    std::vector<ThreadResult> t_results(num_threads);
    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    auto start_time = std::chrono::high_resolution_clock::now();

    auto worker = [&](int id)
    {
        uint64_t local_best_len = 0;
        uint64_t local_best_val = 0;
        bool local_ovf = false;

        int check_stop_counter = 0;

        for (uint64_t i = id + 1; i <= limit; i += num_threads)
        {

            if ((++check_stop_counter & 0xFF) == 0)
            {
                if (stop_flag.load(std::memory_order_relaxed) || global_overflow.load(std::memory_order_relaxed))
                {
                    return;
                }
            }

            uint64_t len = calc_length(i, local_ovf);

            if (local_ovf)
            {
                global_overflow.store(true, std::memory_order_relaxed);
                return;
            }

            if (len > local_best_len)
            {
                local_best_len = len;
                local_best_val = i;
            }
        }

        t_results[id] = {local_best_len, local_best_val};
    };

    for (unsigned t = 0; t < num_threads; ++t)
    {
        threads.emplace_back(worker, t);
    }

    for (auto& th : threads)
    {
        if (th.joinable()) th.join();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    elapsed_ms_ = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    if (global_overflow.load())
    {
        overflow_ = true;
        stopped_ = true;
        message_ = "Calculation failed: Sequence overflowed 64-bit limit.";
        return;
    }

    if (stop_flag.load())
    {
        stopped_ = true;
        message_ = "Stopped by user.";
        return;
    }

    for (const auto& res : t_results)
    {
        if (res.max_len > best_length_) {
            best_length_ = res.max_len;
            best_start_ = res.max_val;
        } else if (res.max_len == best_length_) {
            if (res.max_val > best_start_) best_start_ = res.max_val;
        }
    }

    message_ = "Best Start Number: " + std::to_string(best_start_) + "\n" +
               "Sequence Length: " + std::to_string(best_length_) + "\n" +
               "Time: " + std::to_string(elapsed_ms_) + " ms";
}

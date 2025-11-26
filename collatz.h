#ifndef __COLLATZ_H__
#define __COLLATZ_H__

#include <cstdint>
#include <string>
#include <atomic>
#include <vector>

class CollatzSolver {
public:
    CollatzSolver();
    ~CollatzSolver();

    void compute(uint64_t limit, unsigned num_threads);

    void request_stop();

    uint64_t best_start() const { return best_start_; };
    uint64_t best_length() const { return best_length_; };
    uint64_t elapsed_ms() const { return elapsed_ms_; };
    bool stopped() const { return stopped_; };
    bool overflow() const { return overflow_; };
    const std::string& message() const { return message_; };

private:
    std::atomic<bool> stop_flag;

    uint64_t best_start_ = 0;
    uint64_t best_length_ = 0;
    bool stopped_ = false;
    bool overflow_ = false;
    uint64_t elapsed_ms_ = 0;
    std::string message_;
};

#endif // __COLLATZ_H__

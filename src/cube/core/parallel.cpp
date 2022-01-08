#include <cube/core/parallel.hpp>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <deque>
#include <condition_variable>

#include <iostream>

using namespace cube;
using namespace cube::core;

namespace
{

struct parallel_for_job
{
    core::parallel_exclusive_range_t range;
    core::parallel_handler_t handler;

    void operator()()
    {
        handler(range);
    }
};

template<typename T>
class job_processor
{
public:
    using job_t = T;

    job_processor(unsigned int num_of_threads = std::thread::hardware_concurrency()) :
        stopping_(false),
        in_flight_(0)
    {
        if (num_of_threads < 1)
            throw std::runtime_error("Number of threads < 1");

        for (unsigned int i = 0; i < num_of_threads; ++i) {
            threads_.emplace_back(std::thread([&]() {
                for (;;) {
                    std::unique_lock lock(mutex_);
                    condition_.wait(lock, [this]() { return !jobs_.empty() || stopping_; });

                    if (stopping_)
                        break;

                    auto job = std::move(jobs_.back());
                    jobs_.pop_back();

                    // Run job without holding the lock
                    try {
                        unlocker unlocker(lock);
                        job();
                    } catch(...) {
                        stop();
                        throw;
                    }

                    in_flight_--;
                    in_flight_condition_.notify_all();
                }
            }));
        }
    }

    ~job_processor()
    {
        stop();
        for (auto & t : threads_)
            t.join();
    }

    bool exited() const
    {
        return stopping_;
    };

    int available_threads() const
    {
        return static_cast<int>(threads_.size());
    }

    void schedule(job_t job)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        jobs_.push_front(std::move(job));
        in_flight_++;
        condition_.notify_all();
    }

    void wait_until_processed()
    {
        std::unique_lock lock(mutex_);
        in_flight_condition_.wait(lock, [this]() { return in_flight_ == 0 || stopping_; });
    }

private:
    template<typename Lock>
    struct unlocker
    {
        using lock_t = Lock;

        unlocker(lock_t & lck) :
            lock(lck)
        { lock.unlock(); }
        ~unlocker() { lock.lock(); }

        lock_t & lock;
    };

    void stop()
    {
        stopping_ = true;
        condition_.notify_all();
        in_flight_condition_.notify_all();
    }

    std::atomic_bool stopping_;
    std::mutex mutex_;
    std::condition_variable condition_;
    std::condition_variable in_flight_condition_;
    std::deque<job_t> jobs_;
    std::vector<std::thread> threads_;
    int in_flight_;
};

void parallel_for_impl(core::parallel_exclusive_range_t range, core::parallel_handler_t handler, double nice_factor)
{
    static std::unique_ptr<job_processor<parallel_for_job>> processor;
    if (!processor || processor->exited())
        processor = std::make_unique<job_processor<parallel_for_job>>();

    int const span = std::abs(core::diff(range));
    int const jobs = map(nice_factor, 0.0, 1.0, std::min(span, processor->available_threads()), 1);

    // If we end up with only one job, just directly execute it on this thread
    if (jobs == 1)
        return handler(range);

    int const step = span / jobs;
    int from = range.from;

    // Last job will take into account that span might not be divisible by the number of jobs
    for (int i = 1; i < jobs; ++i) {
        processor->schedule({{from, from + step}, handler});
        from += step;
    }
    processor->schedule({{from, range.to}, handler});
    processor->wait_until_processed(); // Only safe when this function is not nested
}

} // End of namespace

namespace cube::core
{

void parallel_for(parallel_exclusive_range_t range, parallel_handler_t handler, double nice_factor)
{
    static std::atomic_bool running{false};

    if (diff(range) == 0)
        return;

    if (running)
        handler(range); // Nested calls are not parallelized
    else {
        running = true;
        try {
            parallel_for_impl(std::move(range), std::move(handler), std::clamp(nice_factor, 0.0, 1.0));
            running = false;
        } catch(...) {
            running = false;
            throw;
        }
    }
}

} // End of namespace

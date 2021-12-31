#include <cube/core/parallel.hpp>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <deque>
#include <condition_variable>

using namespace cube;

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
class job_scheduler
{
public:
    using job_t = T;

    job_scheduler(unsigned int num_of_threads = std::thread::hardware_concurrency()) :
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

                    // Run job outside without holding the lock
                    {
                        unlocker unlocker(lock);
                        job();
                    }

                    in_flight_--;
                    in_flight_condition_.notify_all();
                }
            }));
        }
    }

    ~job_scheduler()
    {
        stopping_ = true;
        condition_.notify_all();
        in_flight_condition_.notify_all();
        for (auto & t : threads_)
            t.join();
    }

    int available_threads() const
    {
        return static_cast<int>(threads_.size());
    }

    void post(job_t job)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        jobs_.push_front(std::move(job));
        in_flight_++;
        condition_.notify_all();
    }

    void wait_until_processed()
    {
        std::unique_lock lock(mutex_);
        in_flight_condition_.wait(lock, [this]() { return (jobs_.empty() && in_flight_ == 0) || stopping_; });
    }

private:
    template<typename Lock>
    struct unlocker
    {
        unlocker(Lock & lck) :
            lock(lck)
        { lock.unlock(); }
        ~unlocker() { lock.lock(); }

        Lock & lock;
    };

    std::atomic_bool stopping_;
    std::mutex mutex_;
    std::condition_variable condition_;
    std::condition_variable in_flight_condition_;
    std::deque<job_t> jobs_;
    std::vector<std::thread> threads_;
    int in_flight_;
};

void parallel_for_impl(core::parallel_exclusive_range_t range, core::parallel_handler_t handler)
{
    static job_scheduler<parallel_for_job> scheduler;

    int const span = std::abs(core::span(range));
    int const jobs = std::min(span, scheduler.available_threads());
    int const step = span / jobs;
    int from = range.from;

    // Last job will take into account that span might not be divisible by jobs
    for (int i = 1; i < jobs; ++i) {
        scheduler.post({{from, from + step}, handler});
        from += step;
    }
    scheduler.post({{from, range.to}, handler});
    scheduler.wait_until_processed(); // Only safe when this function is not nested
}

} // End of namespace

namespace cube::core
{

void parallel_for(parallel_exclusive_range_t range, parallel_handler_t handler)
{
    static std::atomic_bool running{false};

    if (span(range) == 0)
        return;

    if (running)
        handler(range); // Nested calls are not parallelized
    else {
        running = true;
        try {
            parallel_for_impl(std::move(range), std::move(handler));
            running = false;
        } catch(...) {
            running = false;
            throw;
        }
    }
}

} // End of namespace


#pragma once

#include <experimental/optional>
#include <functional>
#include <future>
#include <memory>
#include <queue>

namespace std
{
    // Pretend we have C++17
    template<typename T>
    using optional = experimental::optional<T>;
}

namespace dispatcher
{
    class task_queue : public std::enable_shared_from_this<task_queue>
    {
    public:
        // Add work to the queue, represented as any sort of parameterless function-like object. The result is returned as a waitable future.
        template<typename TFunctor>
        auto enque(TFunctor&& f)
        {
            std::packaged_task<decltype(f())()> task{ std::forward<TFunctor>(f) };
            auto future = task.get_future();
            enque_f(std::make_unique<typed_task<decltype(f())>>(std::move(task)));
            return future;
        }

        enum class process_result
        {
            finished,
            interrupted,
        };

        // Process work from the queue, until all work is finished or processing is interrupted. (Return value indicates which.) This can be run synchronously, or from a worker thread, or simultaneously from multiple threads.
        process_result process();

        // Convenience method to spawn a new thread and start it processing the queue. Can be called multiple times to spawn a pool of multiple threads.
        std::future<process_result> process_on_new_thread();

        // Signal the queue to finish, once all enqueued work is done
        void finish();

        // Signal the queue to interrupt processing. All tasks actively being processed will complete, but no further tasks will be processed.
        void interrupt();

    private:
        // A simple wrapper class for type erasure (the processing queue just needs to invoke tasks, it doesn't see the return type passed to the waiting future)
        class task
        {
        public:
            virtual ~task() {}
            virtual void operator()() = 0;
        };

        template<typename T>
        class typed_task : public task
        {
        public:
            typed_task(std::packaged_task<T()>&& task) : _task{ std::move(task) } {}
            virtual void operator()() override { _task(); }
        private:
            std::packaged_task<T()> _task;
        };

        std::mutex _mutex;
        std::condition_variable _cv;
        std::optional<process_result> _result;
        std::queue<std::unique_ptr<task>> _tasks;

        void enque_f(std::unique_ptr<task>&& t);
    };
}

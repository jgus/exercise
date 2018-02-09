
#include "dispatcher/dispatcher.hpp"

#include <cassert>

namespace dispatcher
{
    task_queue::process_result task_queue::process()
    {
        while(true)
        {
            std::unique_ptr<task> t;
            {
                std::unique_lock<std::mutex> guard{ _mutex };
                while (_tasks.empty() && !_result)
                {
                    _cv.wait(guard);
                }
                if (_result == process_result::interrupted) { return process_result::interrupted; }
                if (!_tasks.empty())
                {
                    t = std::move(_tasks.front());
                    _tasks.pop();
                }
                else
                {
                    assert(_result == process_result::finished);
                    return process_result::finished;
                }
            }
            assert(t);
            (*t)();
        }
    }

    std::future<task_queue::process_result> task_queue::process_on_new_thread()
    {
        std::promise<process_result> promise;
        auto future = promise.get_future();
        auto thread = std::thread{ [shared = shared_from_this(), promise = std::move(promise)]() mutable
        {
            promise.set_value_at_thread_exit(shared->process());
        }};
        thread.detach();
        return future;
    }

    void task_queue::finish()
    {
        std::unique_lock<std::mutex> guard{ _mutex };
        _result = process_result::finished;
        _cv.notify_all();
    }

    void task_queue::interrupt()
    {
        std::unique_lock<std::mutex> guard{ _mutex };
        _result = process_result::interrupted;
        _cv.notify_all();
    }

    void task_queue::enque_f(std::unique_ptr<task>&& t)
    {
        std::unique_lock<std::mutex> guard{ _mutex };
        _tasks.push(std::move(t));
        _cv.notify_one();
    }
}

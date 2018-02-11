
#include <gtest/gtest.h>

#include "dispatcher/dispatcher.hpp"

using namespace std::chrono_literals;

TEST(Dispatcher, SingleSync)
{
    auto queue = std::make_shared<dispatcher::task_queue>();
    auto result = queue->enque([](){ return 42; });
    queue->finish();
    ASSERT_EQ(queue->process(), dispatcher::task_queue::process_result::finished);
    ASSERT_EQ(result.get(), 42);
}

TEST(Dispatcher, MultiSync)
{
    auto queue = std::make_shared<dispatcher::task_queue>();
    auto result1 = queue->enque([](){ return 42; });
    auto result2 = queue->enque([](){ return 1337; });
    auto result3 = queue->enque([](){ return 1; });
    queue->finish();
    ASSERT_EQ(queue->process(), dispatcher::task_queue::process_result::finished);
    ASSERT_EQ(result1.get(), 42);
    ASSERT_EQ(result2.get(), 1337);
    ASSERT_EQ(result3.get(), 1);
}

TEST(Dispatcher, InterruptSync)
{
    auto queue = std::make_shared<dispatcher::task_queue>();
    auto result = queue->enque([](){ return 42; });
    queue->interrupt();
    ASSERT_EQ(queue->process(), dispatcher::task_queue::process_result::interrupted);
    ASSERT_EQ(result.wait_for(std::chrono::seconds::zero()), std::future_status::timeout);
}

TEST(Dispatcher, SingleAsync)
{
    auto queue = std::make_shared<dispatcher::task_queue>();
    auto result = queue->enque([](){ std::this_thread::sleep_for(20ms); return 42; });
    queue->finish();
    auto process_result = queue->process_on_new_thread();
    ASSERT_EQ(result.get(), 42);
    ASSERT_EQ(process_result.get(), dispatcher::task_queue::process_result::finished);
}

TEST(Dispatcher, MultiAsync)
{
    auto queue = std::make_shared<dispatcher::task_queue>();
    auto result1 = queue->enque([](){ std::this_thread::sleep_for(20ms); return 42; });
    auto result2 = queue->enque([](){ std::this_thread::sleep_for(20ms); return 1337; });
    auto result3 = queue->enque([](){ std::this_thread::sleep_for(20ms); return 1; });
    queue->finish();
    auto process_result = queue->process_on_new_thread();
    ASSERT_EQ(result1.get(), 42);
    ASSERT_EQ(result2.get(), 1337);
    ASSERT_EQ(result3.get(), 1);
    ASSERT_EQ(process_result.get(), dispatcher::task_queue::process_result::finished);
}

TEST(Dispatcher, MultiPool)
{
    auto queue = std::make_shared<dispatcher::task_queue>();
    auto result1 = queue->enque([](){ std::this_thread::sleep_for(20ms); return 42; });
    auto result2 = queue->enque([](){ std::this_thread::sleep_for(20ms); return 1337; });
    auto result3 = queue->enque([](){ std::this_thread::sleep_for(20ms); return 1; });
    queue->finish();
    auto process_result1 = queue->process_on_new_thread();
    auto process_result2 = queue->process_on_new_thread();
    auto process_result3 = queue->process_on_new_thread();
    ASSERT_EQ(result1.get(), 42);
    ASSERT_EQ(result2.get(), 1337);
    ASSERT_EQ(result3.get(), 1);
    ASSERT_EQ(process_result1.get(), dispatcher::task_queue::process_result::finished);
    ASSERT_EQ(process_result2.get(), dispatcher::task_queue::process_result::finished);
    ASSERT_EQ(process_result3.get(), dispatcher::task_queue::process_result::finished);
}

TEST(Dispatcher, InterruptPool)
{
    auto queue = std::make_shared<dispatcher::task_queue>();
    std::vector<std::future<void>> results;
    for (int i = 0; i < 16; ++i)
    {
        results.push_back(queue->enque([](){ std::this_thread::sleep_for(20ms); }));
    }
    std::vector<std::future<dispatcher::task_queue::process_result>> process_results;
    for (int i = 0; i < 4; ++i)
    {
        process_results.push_back(queue->process_on_new_thread());
    }

    std::this_thread::sleep_for(40ms);

    queue->interrupt();

    for (auto& process_result : process_results)
    {
        ASSERT_EQ(process_result.get(), dispatcher::task_queue::process_result::interrupted);
    }

    int ready_count = 0;
    int waiting_count = 0;
    for (auto& result : results)
    {
        if (result.wait_for(0s) == std::future_status::ready)
        {
            ++ready_count;
        }
        else
        {
            ++waiting_count;
        }
    }
    ASSERT_GE(ready_count, 4);
    ASSERT_GE(waiting_count, 4);
}

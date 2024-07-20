#pragma once

namespace Red
{
namespace Detail
{
struct WaitingContext
{
    std::mutex mutex;
    std::condition_variable cv;
    bool finished{false};
};
}

template<typename W>
inline void WaitForQueue(JobQueue& aQueue, const W& aTimeout)
{
    auto context = std::make_shared<Detail::WaitingContext>();

    aQueue.Dispatch([context]() {
        context->finished = true;
        context->cv.notify_all();
    });

    std::unique_lock lock(context->mutex);
    context->cv.wait_for(lock, aTimeout, [context]() { return context->finished; });
}

template<typename W>
inline void WaitForJob(const JobHandle& aJob, const W& aTimeout)
{
    JobQueue queue;
    queue.Wait(aJob);
    WaitForQueue(queue, aTimeout);
}

template<template<typename, typename...> typename V, typename R, typename W, typename... A>
inline void WaitForJobs(const V<R, A...>& aJobs, const W& aTimeout)
{
    JobQueue queue;
    for (const auto& job : aJobs)
    {
        queue.Wait(job);
    }
    WaitForQueue(queue, aTimeout);
}
}

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
inline void WaitForJob(JobHandle& aJob, const W& aTimeout)
{
    auto context = std::make_shared<Detail::WaitingContext>();

    JobQueue queue;
    queue.Wait(aJob);
    queue.Dispatch([context]() {
        context->finished = true;
        context->cv.notify_all();
    });

    std::unique_lock lock(context->mutex);
    context->cv.wait_for(lock, aTimeout, [context]() { return context->finished; });
}

template<typename W>
inline void WaitForQueue(JobQueue& aQueue, const W& aTimeout)
{
    WaitForJob(aQueue.unk10, aTimeout);
}
}

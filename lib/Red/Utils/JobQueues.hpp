#pragma once

namespace Red
{
template<typename W>
inline void WaitForJob(JobHandle& aJob, const W& aTimeout)
{
    std::mutex mutex;
    std::unique_lock lock(mutex);
    std::condition_variable cv;

    JobQueue queue;
    queue.Wait(aJob);
    queue.Dispatch([&lock, &cv]() {
        lock.release();
        cv.notify_all();
    });

    cv.wait_for(lock, aTimeout);

    if (lock.owns_lock())
    {
        cv.wait_for(lock, aTimeout);
    }

    if (!lock || !lock.owns_lock())
    {
        mutex.unlock();
    }
}

template<typename W>
inline void WaitForQueue(JobQueue& aQueue, const W& aTimeout)
{
    WaitForJob(aQueue.unk10, aTimeout);
}
}

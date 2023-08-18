#pragma once

#include "Core/Raw.hpp"
#include "Red/Addresses.hpp"

namespace Red
{
template<typename T, typename W>
inline void WaitForResource(SharedPtr<ResourceToken<T>>& aToken, W aTimeout)
{
    if (!aToken->IsLoaded() && !aToken->IsFailed())
    {
        std::mutex mutex;
        std::unique_lock lock(mutex);
        std::condition_variable cv;
        aToken->OnLoaded([&lock, &cv](Handle<T>&)
                         {
                             lock.release();
                             cv.notify_all();
                         });
        cv.wait_for(lock, aTimeout);
        mutex.unlock();
    }
}

inline void WaitForResources()
{
}
}

namespace Raw::ResourceLoader
{
constexpr auto OnUpdate = Core::RawFunc<
    /* addr = */ Red::Addresses::ResourceLoader_OnUpdate,
    /* type = */ void (*)(Red::ResourceLoader& aLoader)>();
}

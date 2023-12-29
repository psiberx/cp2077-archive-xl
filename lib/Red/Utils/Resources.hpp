#pragma once

#include "JobQueues.hpp"

namespace Red
{
namespace Detail
{
template<typename R>
struct ResourceTraits;

template<typename T>
struct ResourceTraits<SharedPtr<ResourceToken<T>>>
{
    inline static bool IsFinished(const SharedPtr<ResourceToken<T>>& aToken)
    {
        return aToken->IsLoaded() || aToken->IsFailed();
    }

    template<template<typename, typename...> typename V, typename... A>
    inline static bool AllFinished(const V<SharedPtr<ResourceToken<T>>, A...>& aTokens)
    {
        return std::ranges::all_of(aTokens, &IsFinished);
    }

    inline static JobHandle& GetJobHandle(const SharedPtr<ResourceToken<T>>& aToken)
    {
        return aToken->job;
    }

    inline static ResourcePath GetPath(const SharedPtr<ResourceToken<T>>& aToken)
    {
        return aToken->path;
    }
};

template<typename T>
struct ResourceTraits<ResourceReference<T>>
{
    inline static bool IsFinished(const ResourceReference<T>& aReference)
    {
        return aReference.token->IsLoaded() || aReference.token->IsFailed();
    }

    template<template<typename, typename...> typename V, typename... A>
    inline static bool AllFinished(const V<ResourceReference<T>, A...>& aReferences)
    {
        return std::ranges::all_of(aReferences, &IsFinished);
    }

    inline static JobHandle& GetJobHandle(const ResourceReference<T>& aReference)
    {
        return aReference.token->job;
    }

    inline static ResourcePath GetPath(const ResourceReference<T>& aReference)
    {
        return aReference.token->path;
    }
};
}

template<typename R, typename W>
inline void WaitForResource(const R& aResource, const W& aTimeout)
{
    using Trait = Detail::ResourceTraits<R>;

    if (!Trait::IsFinished(aResource))
    {
        WaitForJob(Trait::GetJobHandle(aResource), aTimeout);
    }
}

template<template<typename, typename...> typename V, typename R, typename W, typename... A>
inline void WaitForResources(const V<R, A...>& aResources, const W& aTimeout)
{
    using Trait = Detail::ResourceTraits<R>;

    if (!Trait::AllFinished(aResources))
    {
        JobQueue queue;
        for (const auto& resource : aResources)
        {
            queue.Wait(Trait::GetJobHandle(resource));
        }
        WaitForQueue(queue, aTimeout);
    }
}
}

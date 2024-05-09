#pragma once

#include "Core/Hooking/Detail.hpp"
#include "Core/Hooking/HookingDriver.hpp"

namespace Core::Hook
{
template<typename TTarget, typename TCallback, HookFlow TFlow = HookFlow::Original, HookRun TRun = HookRun::Default>
requires Detail::HookFlowTraits<TFlow, TTarget, TCallback>::IsCompatible
inline auto Attach(TCallback&& aCallback, typename TTarget::Callable* aOriginal = nullptr)
{
    using Wrapper = Detail::HookWrapper<TCallback>;
    using Handler = Detail::HookHandler<TTarget, Wrapper, TFlow, TRun>;
    return Handler::Attach(HookingDriver::GetDefault(), Wrapper(std::forward<TCallback>(aCallback)), aOriginal);
}

template<typename TTarget, typename TCallback, HookRun TRun = HookRun::Default>
requires Detail::HookFlowTraits<HookFlow::Before, TTarget, TCallback>::IsCompatible
inline auto Before(TCallback&& aCallback)
{
    using Wrapper = Detail::HookWrapper<TCallback>;
    using Handler = Detail::HookHandler<TTarget, Wrapper, HookFlow::Before, TRun>;
    return Handler::Attach(HookingDriver::GetDefault(), Wrapper(std::forward<TCallback>(aCallback)));
}

template<typename TTarget, typename TCallback>
requires Detail::HookFlowTraits<HookFlow::Before, TTarget, TCallback>::IsCompatible
inline auto OnceBefore(TCallback&& aCallback)
{
    return Before<TTarget, TCallback, HookRun::Once>(std::forward<TCallback>(aCallback));
}

template<typename TTarget, typename TCallback, HookRun TRun = HookRun::Default>
requires Detail::HookFlowTraits<HookFlow::After, TTarget, TCallback>::IsCompatible
inline auto After(TCallback&& aCallback)
{
    using Wrapper = Detail::HookWrapper<TCallback>;
    using Handler = Detail::HookHandler<TTarget, Wrapper, HookFlow::After, TRun>;
    return Handler::Attach(HookingDriver::GetDefault(), Wrapper(std::forward<TCallback>(aCallback)));
}

template<typename TTarget, typename TCallback>
requires Detail::HookFlowTraits<HookFlow::After, TTarget, TCallback>::IsCompatible
inline auto OnceAfter(TCallback&& aCallback)
{
    return After<TTarget, TCallback, HookRun::Once>(std::forward<TCallback>(aCallback));
}

template<typename TTarget, typename TCallback, HookRun TRun = HookRun::Default>
requires Detail::HookFlowTraits<HookFlow::Wrap, TTarget, TCallback>::IsCompatible
inline auto Wrap(TCallback&& aCallback)
{
    using Wrapper = Detail::HookWrapper<TCallback>;
    using Handler = Detail::HookHandler<TTarget, Wrapper, HookFlow::Wrap, TRun>;
    return Handler::Attach(HookingDriver::GetDefault(), Wrapper(std::forward<TCallback>(aCallback)));
}

template<typename TTarget, typename TCallback>
requires Detail::HookFlowTraits<HookFlow::Wrap, TTarget, TCallback>::IsCompatible
inline auto WrapOnce(TCallback&& aCallback)
{
    return Wrap<TTarget, TCallback, HookRun::Once>(std::forward<TCallback>(aCallback));
}

template<typename TTarget>
inline auto Detach()
{
    using Instance = Detail::HookInstance<TTarget>;
    return Instance::Detach();
}

template<RawFunc TTarget, typename TCallback, HookFlow TFlow = HookFlow::Original, HookRun TRun = HookRun::Default>
requires Detail::HookFlowTraits<TFlow, decltype(TTarget), TCallback>::IsCompatible
inline static auto Attach(TCallback&& aCallback, typename decltype(TTarget)::Callable* aOriginal = nullptr)
{
    return Attach<decltype(TTarget), TCallback, TFlow, TRun>(std::forward<TCallback>(aCallback), aOriginal);
}

template<RawFunc TTarget, typename TCallback, HookRun TRun = HookRun::Default>
requires Detail::HookFlowTraits<HookFlow::Before, decltype(TTarget), TCallback>::IsCompatible
inline static auto Before(TCallback&& aCallback)
{
    return Before<decltype(TTarget), TCallback, TRun>(std::forward<TCallback>(aCallback));
}

template<RawFunc TTarget, typename TCallback>
requires Detail::HookFlowTraits<HookFlow::Before, decltype(TTarget), TCallback>::IsCompatible
inline static auto OnceBefore(TCallback&& aCallback)
{
    return Before<decltype(TTarget), TCallback, HookRun::Once>(std::forward<TCallback>(aCallback));
}

template<RawFunc TTarget, typename TCallback, HookRun TRun = HookRun::Default>
requires Detail::HookFlowTraits<HookFlow::After, decltype(TTarget), TCallback>::IsCompatible
inline static auto After(TCallback&& aCallback)
{
    return After<decltype(TTarget), TCallback, TRun>(std::forward<TCallback>(aCallback));
}

template<RawFunc TTarget, typename TCallback>
requires Detail::HookFlowTraits<HookFlow::After, decltype(TTarget), TCallback>::IsCompatible
inline static auto OnceAfter(TCallback&& aCallback)
{
    return After<decltype(TTarget), TCallback, HookRun::Once>(std::forward<TCallback>(aCallback));
}

template<RawFunc TTarget, typename TCallback, HookRun TRun = HookRun::Default>
requires Detail::HookFlowTraits<HookFlow::Wrap, decltype(TTarget), TCallback>::IsCompatible
inline static auto Wrap(TCallback&& aCallback)
{
    return Wrap<decltype(TTarget), TCallback, TRun>(std::forward<TCallback>(aCallback));
}

template<RawFunc TTarget, typename TCallback>
requires Detail::HookFlowTraits<HookFlow::Wrap, decltype(TTarget), TCallback>::IsCompatible
inline static auto WrapOnce(TCallback&& aCallback)
{
    return Wrap<decltype(TTarget), TCallback, HookRun::Once>(std::forward<TCallback>(aCallback));
}

template<RawFunc TTarget>
inline static auto Detach()
{
    return Detach<decltype(TTarget)>();
}
}

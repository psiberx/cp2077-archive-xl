#pragma once

#include "Core/Hooking/Detail.hpp"
#include "Core/Hooking/HookingDriver.hpp"

namespace Core::Hook
{
template<typename TTarget, typename TCallback, HookFlow TFlow = HookFlow::Original, HookRun TRun = HookRun::Default>
requires Detail::HookFlowTraits<TFlow, TTarget, TCallback>::IsCompatible
inline bool Attach(TCallback&& aCallback, typename TTarget::Callable* aOriginal = nullptr)
{
    using Wrapper = Detail::HookWrapper<TCallback>;
    using Handler = Detail::HookHandler<TTarget, Wrapper, TFlow, TRun>;
    return Handler::Attach(HookingDriver::GetDefault(), Wrapper(std::move(aCallback)), aOriginal);
}

template<typename TTarget, typename TCallback, HookRun TRun = HookRun::Default>
requires Detail::HookFlowTraits<HookFlow::Before, TTarget, TCallback>::IsCompatible
inline bool Before(TCallback&& aCallback)
{
    using Wrapper = Detail::HookWrapper<TCallback>;
    using Handler = Detail::HookHandler<TTarget, Wrapper, HookFlow::Before, TRun>;
    return Handler::Attach(HookingDriver::GetDefault(), Wrapper(std::move(aCallback)));
}

template<typename TTarget, typename TCallback>
requires Detail::HookFlowTraits<HookFlow::Before, TTarget, TCallback>::IsCompatible
inline bool BeforeOnce(TCallback&& aCallback)
{
    return Before<TTarget, TCallback, HookRun::Once>(std::move(aCallback));
}

template<typename TTarget, typename TCallback, HookRun TRun = HookRun::Default>
requires Detail::HookFlowTraits<HookFlow::After, TTarget, TCallback>::IsCompatible
inline bool After(TCallback&& aCallback)
{
    using Wrapper = Detail::HookWrapper<TCallback>;
    using Handler = Detail::HookHandler<TTarget, Wrapper, HookFlow::After, TRun>;
    return Handler::Attach(HookingDriver::GetDefault(), Wrapper(std::move(aCallback)));
}

template<typename TTarget, typename TCallback>
requires Detail::HookFlowTraits<HookFlow::After, TTarget, TCallback>::IsCompatible
inline bool AfterOnce(TCallback&& aCallback)
{
    return After<TTarget, TCallback, HookRun::Once>(std::move(aCallback));
}

template<typename TTarget, typename TCallback, HookRun TRun = HookRun::Default>
requires Detail::HookFlowTraits<HookFlow::Wrap, TTarget, TCallback>::IsCompatible
inline bool Wrap(TCallback&& aCallback)
{
    using Wrapper = Detail::HookWrapper<TCallback>;
    using Handler = Detail::HookHandler<TTarget, Wrapper, HookFlow::Wrap, TRun>;
    return Handler::Attach(HookingDriver::GetDefault(), Wrapper(std::move(aCallback)));
}

template<typename TTarget, typename TCallback>
requires Detail::HookFlowTraits<HookFlow::Wrap, TTarget, TCallback>::IsCompatible
inline bool WrapOnce(TCallback&& aCallback)
{
    return Wrap<TTarget, TCallback, HookRun::Once>(std::move(aCallback));
}

template<class TTarget>
inline bool Detach()
{
    using Instance = Detail::HookInstance<TTarget>;
    return Instance::Detach();
}
}

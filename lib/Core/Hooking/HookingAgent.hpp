#pragma once

#include "Core/Hooking/Detail.hpp"
#include "Core/Hooking/HookingDriver.hpp"

namespace Core
{
class HookingAgent
{
protected:
    template<typename TTarget, typename TCallback, HookFlow TFlow = HookFlow::Original, HookRun TRun = HookRun::Default>
    requires Detail::HookFlowTraits<TFlow, TTarget, TCallback>::IsCompatibleNonMember
    inline static auto Hook(TCallback&& aCallback, typename TTarget::Callable* aOriginal = nullptr)
    {
        using Wrapper = Detail::HookWrapper<TCallback>;
        using Handler = Detail::HookHandler<TTarget, Wrapper, TFlow, TRun>;
        return Handler::Attach(HookingDriver::GetDefault(), Wrapper(std::forward<TCallback>(aCallback)), aOriginal);
    }

    template<typename TTarget, typename TCallback, HookRun TRun = HookRun::Default>
    requires Detail::HookFlowTraits<HookFlow::Before, TTarget, TCallback>::IsCompatibleNonMember
    inline static auto HookBefore(TCallback&& aCallback)
    {
        using Wrapper = Detail::HookWrapper<TCallback>;
        using Handler = Detail::HookHandler<TTarget, Wrapper, HookFlow::Before, TRun>;
        return Handler::Attach(HookingDriver::GetDefault(), Wrapper(std::forward<TCallback>(aCallback)));
    }

    template<typename TTarget, typename TCallback>
    requires Detail::HookFlowTraits<HookFlow::Before, TTarget, TCallback>::IsCompatibleNonMember
    inline static auto HookOnceBefore(TCallback&& aCallback)
    {
        return HookBefore<TTarget, TCallback, HookRun::Once>(std::forward<TCallback>(aCallback));
    }

    template<typename TTarget, typename TCallback, HookRun TRun = HookRun::Default>
    requires Detail::HookFlowTraits<HookFlow::After, TTarget, TCallback>::IsCompatibleNonMember
    inline static auto HookAfter(TCallback&& aCallback)
    {
        using Wrapper = Detail::HookWrapper<TCallback>;
        using Handler = Detail::HookHandler<TTarget, Wrapper, HookFlow::After, TRun>;
        return Handler::Attach(HookingDriver::GetDefault(), Wrapper(std::forward<TCallback>(aCallback)));
    }

    template<typename TTarget, typename TCallback>
    requires Detail::HookFlowTraits<HookFlow::After, TTarget, TCallback>::IsCompatibleNonMember
    inline static auto HookOnceAfter(TCallback&& aCallback)
    {
        return HookAfter<TTarget, TCallback, HookRun::Once>(std::forward<TCallback>(aCallback));
    }

    template<typename TTarget, typename TCallback, HookRun TRun = HookRun::Default>
    requires Detail::HookFlowTraits<HookFlow::Wrap, TTarget, TCallback>::IsCompatibleNonMember
    inline static auto HookWrap(TCallback&& aCallback)
    {
        using Wrapper = Detail::HookWrapper<TCallback>;
        using Handler = Detail::HookHandler<TTarget, Wrapper, HookFlow::Wrap, TRun>;
        return Handler::Attach(HookingDriver::GetDefault(), Wrapper(std::forward<TCallback>(aCallback)));
    }

    template<typename TTarget, typename TCallback>
    requires Detail::HookFlowTraits<HookFlow::Wrap, TTarget, TCallback>::IsCompatibleNonMember
    inline static auto HookWrapOnce(TCallback&& aCallback)
    {
        return HookWrap<TTarget, TCallback, HookRun::Once>(std::forward<TCallback>(aCallback));
    }

    template<typename TTarget, typename TCallback, HookFlow TFlow = HookFlow::Original, HookRun TRun = HookRun::Default>
    requires Detail::HookFlowTraits<TFlow, TTarget, TCallback>::IsCompatibleMember
    inline auto Hook(const TCallback& aCallback, typename TTarget::Callable* aOriginal = nullptr)
    {
        using Wrapper = Detail::HookWrapper<TCallback>;
        using Handler = Detail::HookHandler<TTarget, Wrapper, TFlow, TRun>;
        return Handler::Attach(HookingDriver::GetDefault(), Wrapper(aCallback, this), aOriginal);
    }

    template<typename TTarget, typename TCallback, HookRun TRun = HookRun::Default>
    requires Detail::HookFlowTraits<HookFlow::Before, TTarget, TCallback>::IsCompatibleMember
    inline auto HookBefore(const TCallback& aCallback)
    {
        using Wrapper = Detail::HookWrapper<TCallback>;
        using Handler = Detail::HookHandler<TTarget, Wrapper, HookFlow::Before, TRun>;
        return Handler::Attach(HookingDriver::GetDefault(), Wrapper(aCallback, this));
    }

    template<typename TTarget, typename TCallback>
    requires Detail::HookFlowTraits<HookFlow::Before, TTarget, TCallback>::IsCompatibleMember
    inline auto HookOnceBefore(const TCallback& aCallback)
    {
        return HookBefore<TTarget, TCallback, HookRun::Once>(aCallback);
    }

    template<typename TTarget, typename TCallback, HookRun TRun = HookRun::Default>
    requires Detail::HookFlowTraits<HookFlow::After, TTarget, TCallback>::IsCompatibleMember
    inline auto HookAfter(const TCallback& aCallback)
    {
        using Wrapper = Detail::HookWrapper<TCallback>;
        using Handler = Detail::HookHandler<TTarget, Wrapper, HookFlow::After, TRun>;
        return Handler::Attach(HookingDriver::GetDefault(), Wrapper(aCallback, this));
    }

    template<typename TTarget, typename TCallback>
    requires Detail::HookFlowTraits<HookFlow::After, TTarget, TCallback>::IsCompatibleMember
    inline auto HookOnceAfter(const TCallback& aCallback)
    {
        return HookAfter<TTarget, TCallback, HookRun::Once>(aCallback);
    }

    template<typename TTarget, typename TCallback, HookRun TRun = HookRun::Default>
    requires Detail::HookFlowTraits<HookFlow::Wrap, TTarget, TCallback>::IsCompatibleMember
    inline auto HookWrap(const TCallback& aCallback)
    {
        using Wrapper = Detail::HookWrapper<TCallback>;
        using Handler = Detail::HookHandler<TTarget, Wrapper, HookFlow::Wrap, TRun>;
        return Handler::Attach(HookingDriver::GetDefault(), Wrapper(aCallback, this));
    }

    template<typename TTarget, typename TCallback>
    requires Detail::HookFlowTraits<HookFlow::Wrap, TTarget, TCallback>::IsCompatibleMember
    inline auto HookWrapOnce(const TCallback& aCallback)
    {
        return HookWrap<TTarget, TCallback, HookRun::Once>(aCallback);
    }

    template<typename TTarget>
    inline static auto Unhook()
    {
        using Instance = Detail::HookInstance<TTarget>;
        return Instance::Detach();
    }

    template<typename TTarget>
    inline static auto IsHooked()
    {
        using Instance = Detail::HookInstance<TTarget>;
        return Instance::IsAttached();
    }

    template<RawFunc TTarget, typename TCallback, HookFlow TFlow = HookFlow::Original, HookRun TRun = HookRun::Default>
    requires Detail::HookFlowTraits<TFlow, decltype(TTarget), TCallback>::IsCompatibleNonMember
    inline static auto Hook(TCallback&& aCallback, typename decltype(TTarget)::Callable* aOriginal = nullptr)
    {
        return Hook<decltype(TTarget), TCallback, TFlow, TRun>(std::forward<TCallback>(aCallback), aOriginal);
    }

    template<RawFunc TTarget, typename TCallback, HookRun TRun = HookRun::Default>
    requires Detail::HookFlowTraits<HookFlow::Before, decltype(TTarget), TCallback>::IsCompatibleNonMember
    inline static auto HookBefore(TCallback&& aCallback)
    {
        return HookBefore<decltype(TTarget), TCallback, TRun>(std::forward<TCallback>(aCallback));
    }

    template<RawFunc TTarget, typename TCallback>
    requires Detail::HookFlowTraits<HookFlow::Before, decltype(TTarget), TCallback>::IsCompatibleNonMember
    inline static auto HookOnceBefore(TCallback&& aCallback)
    {
        return HookBefore<decltype(TTarget), TCallback, HookRun::Once>(std::forward<TCallback>(aCallback));
    }

    template<RawFunc TTarget, typename TCallback, HookRun TRun = HookRun::Default>
    requires Detail::HookFlowTraits<HookFlow::After, decltype(TTarget), TCallback>::IsCompatibleNonMember
    inline static auto HookAfter(TCallback&& aCallback)
    {
        return HookAfter<decltype(TTarget), TCallback, TRun>(std::forward<TCallback>(aCallback));
    }

    template<RawFunc TTarget, typename TCallback>
    requires Detail::HookFlowTraits<HookFlow::After, decltype(TTarget), TCallback>::IsCompatibleNonMember
    inline static auto HookOnceAfter(TCallback&& aCallback)
    {
        return HookAfter<decltype(TTarget), TCallback, HookRun::Once>(std::forward<TCallback>(aCallback));
    }

    template<RawFunc TTarget, typename TCallback, HookRun TRun = HookRun::Default>
    requires Detail::HookFlowTraits<HookFlow::Wrap, decltype(TTarget), TCallback>::IsCompatibleNonMember
    inline static auto HookWrap(TCallback&& aCallback)
    {
        return HookWrap<decltype(TTarget), TCallback, TRun>(std::forward<TCallback>(aCallback));
    }

    template<RawFunc TTarget, typename TCallback>
    requires Detail::HookFlowTraits<HookFlow::Wrap, decltype(TTarget), TCallback>::IsCompatibleNonMember
    inline static auto HookWrapOnce(TCallback&& aCallback)
    {
        return HookWrap<decltype(TTarget), TCallback, HookRun::Once>(std::forward<TCallback>(aCallback));
    }

    template<RawFunc TTarget, typename TCallback, HookFlow TFlow = HookFlow::Original, HookRun TRun = HookRun::Default>
    requires Detail::HookFlowTraits<TFlow, decltype(TTarget), TCallback>::IsCompatibleMember
    inline auto Hook(const TCallback& aCallback, typename decltype(TTarget)::Callable* aOriginal = nullptr)
    {
        return Hook<decltype(TTarget), TCallback, TFlow, TRun>(aCallback, aOriginal);
    }

    template<RawFunc TTarget, typename TCallback, HookRun TRun = HookRun::Default>
    requires Detail::HookFlowTraits<HookFlow::Before, decltype(TTarget), TCallback>::IsCompatibleMember
    inline auto HookBefore(const TCallback& aCallback)
    {
        return HookBefore<decltype(TTarget), TCallback, TRun>(aCallback);
    }

    template<RawFunc TTarget, typename TCallback>
    requires Detail::HookFlowTraits<HookFlow::Before, decltype(TTarget), TCallback>::IsCompatibleMember
    inline auto HookOnceBefore(const TCallback& aCallback)
    {
        return HookBefore<decltype(TTarget), TCallback, HookRun::Once>(aCallback);
    }

    template<RawFunc TTarget, typename TCallback, HookRun TRun = HookRun::Default>
    requires Detail::HookFlowTraits<HookFlow::After, decltype(TTarget), TCallback>::IsCompatibleMember
    inline auto HookAfter(const TCallback& aCallback)
    {
        return HookAfter<decltype(TTarget), TCallback, TRun>(aCallback);
    }

    template<RawFunc TTarget, typename TCallback>
    requires Detail::HookFlowTraits<HookFlow::After, decltype(TTarget), TCallback>::IsCompatibleMember
    inline auto HookOnceAfter(const TCallback& aCallback)
    {
        return HookAfter<decltype(TTarget), TCallback, HookRun::Once>(aCallback);
    }

    template<RawFunc TTarget, typename TCallback, HookRun TRun = HookRun::Default>
    requires Detail::HookFlowTraits<HookFlow::Wrap, decltype(TTarget), TCallback>::IsCompatibleMember
    inline auto HookWrap(const TCallback& aCallback)
    {
        return HookWrap<decltype(TTarget), TCallback, TRun>(aCallback);
    }

    template<RawFunc TTarget, typename TCallback>
    requires Detail::HookFlowTraits<HookFlow::Wrap, decltype(TTarget), TCallback>::IsCompatibleMember
    inline auto HookWrapOnce(const TCallback& aCallback)
    {
        return HookWrap<decltype(TTarget), TCallback, HookRun::Once>(aCallback);
    }

    template<RawFunc TTarget>
    inline static auto Unhook()
    {
        return Unhook<decltype(TTarget)>();
    }

    template<RawFunc TTarget>
    inline static auto IsHooked()
    {
        return IsHooked<decltype(TTarget)>();
    }

    static HookingDriver& GetHookingDriver();

private:
    friend HookingDriver;

    static void SetHookingDriver(HookingDriver& aDriver);
};
}

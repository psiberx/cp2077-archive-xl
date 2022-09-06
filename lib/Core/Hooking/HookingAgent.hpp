#pragma once

#include "Core/Hooking/Detail.hpp"
#include "Core/Hooking/Internals.hpp"
#include "Core/Hooking/HookingDriver.hpp"

namespace Core
{
class HookingAgent
{
protected:
    template<typename TTarget, typename TCallback, HookFlow TFlow = HookFlow::Original, HookRun TRun = HookRun::Default>
    requires Detail::HookFlowTraits<TFlow, TTarget, TCallback>::IsCompatibleNotMember
    inline static bool Hook(TCallback&& aCallback, typename TTarget::Callable* aOriginal = nullptr)
    {
        using Wrapper = Detail::HookWrapper<TCallback>;
        using Handler = Detail::HookHandler<TTarget, Wrapper, TFlow, TRun>;
        return Handler::Attach(HookingDriver::GetDefault(), Wrapper(std::move(aCallback)), aOriginal);
    }
    
    template<typename TTarget, typename TCallback, HookRun TRun = HookRun::Default>
    requires Detail::HookFlowTraits<HookFlow::Before, TTarget, TCallback>::IsCompatibleNotMember
    inline static bool HookBefore(TCallback&& aCallback)
    {
        using Wrapper = Detail::HookWrapper<TCallback>;
        using Handler = Detail::HookHandler<TTarget, Wrapper, HookFlow::Before, TRun>;
        return Handler::Attach(HookingDriver::GetDefault(), Wrapper(std::move(aCallback)));
    }
    
    template<typename TTarget, typename TCallback>
    requires Detail::HookFlowTraits<HookFlow::Before, TTarget, TCallback>::IsCompatibleNotMember
    inline static bool HookBeforeOnce(TCallback&& aCallback)
    {
        return HookBefore<TTarget, TCallback, HookRun::Once>(std::move(aCallback));
    }
    
    template<typename TTarget, typename TCallback, HookRun TRun = HookRun::Default>
    requires Detail::HookFlowTraits<HookFlow::After, TTarget, TCallback>::IsCompatibleNotMember
    inline static bool HookAfter(TCallback&& aCallback)
    {
        using Wrapper = Detail::HookWrapper<TCallback>;
        using Handler = Detail::HookHandler<TTarget, Wrapper, HookFlow::After, TRun>;
        return Handler::Attach(HookingDriver::GetDefault(), Wrapper(std::move(aCallback)));
    }
    
    template<typename TTarget, typename TCallback>
    requires Detail::HookFlowTraits<HookFlow::After, TTarget, TCallback>::IsCompatibleNotMember
    inline static bool HookAfterOnce(TCallback&& aCallback)
    {
        return HookAfter<TTarget, TCallback, HookRun::Once>(std::move(aCallback));
    }
    
    template<typename TTarget, typename TCallback, HookRun TRun = HookRun::Default>
    requires Detail::HookFlowTraits<HookFlow::Wrap, TTarget, TCallback>::IsCompatibleNotMember
    inline static bool HookWrap(TCallback&& aCallback)
    {
        using Wrapper = Detail::HookWrapper<TCallback>;
        using Handler = Detail::HookHandler<TTarget, Wrapper, HookFlow::Wrap, TRun>;
        return Handler::Attach(HookingDriver::GetDefault(), Wrapper(std::move(aCallback)));
    }
    
    template<typename TTarget, typename TCallback>
    requires Detail::HookFlowTraits<HookFlow::Wrap, TTarget, TCallback>::IsCompatibleNotMember
    inline static bool HookWrapOnce(TCallback&& aCallback)
    {
        return HookWrap<TTarget, TCallback, HookRun::Once>(std::move(aCallback));
    }

    template<typename TTarget, typename TCallback, HookFlow TFlow = HookFlow::Original, HookRun TRun = HookRun::Default>
    requires Detail::HookFlowTraits<TFlow, TTarget, TCallback>::IsCompatibleMember
    inline bool Hook(const TCallback& aCallback, typename TTarget::Callable* aOriginal = nullptr)
    {
        using Wrapper = Detail::HookWrapper<TCallback>;
        using Handler = Detail::HookHandler<TTarget, Wrapper, TFlow, TRun>;
        return Handler::Attach(HookingDriver::GetDefault(), Wrapper(aCallback, this), aOriginal);
    }
    
    template<typename TTarget, typename TCallback, HookRun TRun = HookRun::Default>
    requires Detail::HookFlowTraits<HookFlow::Before, TTarget, TCallback>::IsCompatibleMember
    inline bool HookBefore(const TCallback& aCallback)
    {
        using Wrapper = Detail::HookWrapper<TCallback>;
        using Handler = Detail::HookHandler<TTarget, Wrapper, HookFlow::Before, TRun>;
        return Handler::Attach(HookingDriver::GetDefault(), Wrapper(aCallback, this));
    }
    
    template<typename TTarget, typename TCallback>
    requires Detail::HookFlowTraits<HookFlow::Before, TTarget, TCallback>::IsCompatibleMember
    inline bool HookBeforeOnce(const TCallback& aCallback)
    {
        return HookBefore<TTarget, TCallback, HookRun::Once>(aCallback);
    }
    
    template<typename TTarget, typename TCallback, HookRun TRun = HookRun::Default>
    requires Detail::HookFlowTraits<HookFlow::After, TTarget, TCallback>::IsCompatibleMember
    inline bool HookAfter(const TCallback& aCallback)
    {
        using Wrapper = Detail::HookWrapper<TCallback>;
        using Handler = Detail::HookHandler<TTarget, Wrapper, HookFlow::After, TRun>;
        return Handler::Attach(HookingDriver::GetDefault(), Wrapper(aCallback, this));
    }
    
    template<typename TTarget, typename TCallback>
    requires Detail::HookFlowTraits<HookFlow::After, TTarget, TCallback>::IsCompatibleMember
    inline bool HookAfterOnce(const TCallback& aCallback)
    {
        return HookAfter<TTarget, TCallback, HookRun::Once>(aCallback);
    }
    
    template<typename TTarget, typename TCallback, HookRun TRun = HookRun::Default>
    requires Detail::HookFlowTraits<HookFlow::Wrap, TTarget, TCallback>::IsCompatibleMember
    inline bool HookWrap(const TCallback& aCallback)
    {
        using Wrapper = Detail::HookWrapper<TCallback>;
        using Handler = Detail::HookHandler<TTarget, Wrapper, HookFlow::Wrap, TRun>;
        return Handler::Attach(HookingDriver::GetDefault(), Wrapper(aCallback, this));
    }
    
    template<typename TTarget, typename TCallback>
    requires Detail::HookFlowTraits<HookFlow::Wrap, TTarget, TCallback>::IsCompatibleMember
    inline bool HookWrapOnce(const TCallback& aCallback)
    {
        return HookWrap<TTarget, TCallback, HookRun::Once>(aCallback);
    }

    template<class TTarget>
    inline static bool Unhook()
    {
        using Instance = Detail::HookInstance<TTarget>;
        return Instance::Detach();
    }

    static HookingDriver& GetHookingDriver();

private:
    friend HookingDriver;

    static void SetHookingDriver(HookingDriver& aDriver);
};
}

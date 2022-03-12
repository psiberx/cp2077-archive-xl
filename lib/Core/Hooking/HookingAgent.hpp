#pragma once

#include "Internals.hpp"
#include "HookingDriver.hpp"

namespace Core
{
class HookingAgent
{
protected:
    template<typename F, typename R, typename... Args,
             typename = std::enable_if_t<std::is_same_v<typename F::type, detail::OriginalFunc<R, Args...>>>>
    static bool Hook(detail::StaticCallback<R, Args...> aCallback,
                     detail::OriginalFunc<R, Args...>* aOriginal = nullptr)
    {
        using Capture = detail::BaseCapture<F>;

        if (Capture::hooked)
            return false;

        detail::OriginalFunc<R, Args...> original;
        const auto address = F::RelocAddr();

        if (!GetHookingDriver().HookAttach(address, aCallback, reinterpret_cast<void**>(&original)))
            return false;

        if (aOriginal)
            *aOriginal = original;

        Capture::hooked = true;

        return true;
    }

    template<typename F, typename C, typename R, typename... Args,
             typename = std::enable_if_t<std::is_same_v<typename F::type, detail::OriginalFunc<R, Args...>>>>
    static bool Hook(C* aContext, detail::MemberCallback<C, R, Args...> aCallback,
                     detail::OriginalFunc<R, Args...>* aOriginal = nullptr)
    {
        using Capture = detail::MemberCapture<F, C, R, Args...>;

        if (Capture::hooked)
            return false;

        const auto address = F::RelocAddr();

        if (!GetHookingDriver().HookAttach(address, &Capture::Handle, reinterpret_cast<void**>(&Capture::original)))
            return false;

        if (aOriginal)
            *aOriginal = Capture::original;

        Capture::callback = aCallback;
        Capture::context = aContext;
        Capture::hooked = true;

        return true;
    }

    template<typename F, typename C, typename R, typename... Args,
             typename = std::enable_if_t<std::is_same_v<typename F::type, detail::OriginalFunc<R, Args...>>>>
    bool Hook(detail::MemberCallback<C, R, Args...> aCallback, detail::OriginalFunc<R, Args...>* aOriginal = nullptr)
    {
        return Hook<F>(static_cast<C*>(this), aCallback, aOriginal);
    }

    template<typename F, typename R, typename... Args,
             typename = std::enable_if_t<std::is_same_v<typename F::type, detail::OriginalFunc<R, Args...>>>>
    bool HookWrap(detail::WrappedCallback<R, Args...> aCallback)
    {
        using Capture = detail::WrappedCapture<F, R, Args...>;

        if (Capture::hooked)
            return false;

        const auto address = F::RelocAddr();

        if (!GetHookingDriver().HookAttach(address, &Capture::Handle, reinterpret_cast<void**>(&Capture::original)))
            return false;

        Capture::callback = aCallback;
        Capture::hooked = true;

        return true;
    }

    template<typename F, typename R, typename... Args,
             typename = std::enable_if_t<std::is_same_v<typename F::type, detail::OriginalFunc<R, Args...>>>>
    bool HookOnce(detail::WrappedCallback<R, Args...> aCallback)
    {
        using Capture = detail::WrappedOnceCapture<F, R, Args...>;

        if (Capture::hooked)
            return false;

        auto& driver = GetHookingDriver();
        const auto address = F::RelocAddr();

        if (!driver.HookAttach(address, &Capture::Handle, reinterpret_cast<void**>(&Capture::original)))
            return false;

        Capture::driver = GetHookingDriver();
        Capture::callback = aCallback;
        Capture::hooked = true;

        return true;
    }

    template<typename F, typename... Args,
             typename R = typename F::return_type,
             typename = std::enable_if_t<std::is_same_v<typename F::type, detail::OriginalFunc<R, Args...>>>>
    bool HookBefore(detail::NoReturnCallback<Args...> aCallback)
    {
        using Capture = detail::BeforeCapture<F, R, Args...>;

        if (Capture::hooked)
            return false;

        const auto address = F::RelocAddr();

        if (!GetHookingDriver().HookAttach(address, &Capture::Handle, reinterpret_cast<void**>(&Capture::original)))
            return false;

        Capture::callback = aCallback;
        Capture::hooked = true;

        return true;
    }

    template<typename F, typename... Args,
             typename R = typename F::return_type,
             typename = std::enable_if_t<std::is_same_v<typename F::type, detail::OriginalFunc<R, Args...>>>>
    bool HookAfter(detail::NoReturnCallback<Args...> aCallback)
    {
        using Capture = detail::AfterCapture<F, R, Args...>;

        if (Capture::hooked)
            return false;

        const auto address = F::RelocAddr();

        if (!GetHookingDriver().HookAttach(address, &Capture::Handle, reinterpret_cast<void**>(&Capture::original)))
            return false;

        Capture::callback = aCallback;
        Capture::hooked = true;

        return true;
    }

    template<typename F>
    bool Unhook()
    {
        using Capture = detail::BaseCapture<F>;

        if (!Capture::hooked)
            return false;

        if (!GetHookingDriver().HookDetach(F::RelocAddr()))
            return false;

        return true;
    }

    static HookingDriver& GetHookingDriver();

private:
    friend HookingDriver;

    static void SetHookingDriver(HookingDriver& aDriver);
};
}

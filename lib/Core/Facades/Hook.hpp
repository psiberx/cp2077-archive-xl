#pragma once

#include "Core/Hooking/Internals.hpp"
#include "Core/Hooking/HookingDriver.hpp"

namespace Core::Hook
{
template<typename F, typename R, typename... Args,
         typename = std::enable_if_t<std::is_same_v<typename F::type, detail::OriginalFunc<R, Args...>>>>
bool Wrap(detail::WrappedCallback<R, Args...> aCallback)
{
    using Capture = detail::WrappedCapture<F, R, Args...>;

    if (Capture::hooked)
        return false;

    auto& driver = HookingDriver::GetDefault();
    const auto address = F::RelocAddr();

    if (!driver.HookAttach(address, &Capture::Handle, reinterpret_cast<void**>(&Capture::original)))
        return false;

    Capture::callback = aCallback;
    Capture::hooked = true;

    return true;
}

template<typename F, typename R, typename... Args,
         typename = std::enable_if_t<std::is_same_v<typename F::type, detail::OriginalFunc<R, Args...>>>>
bool Once(detail::WrappedCallback<R, Args...> aCallback)
{
    using Capture = detail::WrappedOnceCapture<F, R, Args...>;

    if (Capture::hooked)
        return false;

    auto& driver = HookingDriver::GetDefault();
    const auto address = F::RelocAddr();

    if (!driver.HookAttach(address, &Capture::Handle, reinterpret_cast<void**>(&Capture::original)))
        return false;

    Capture::driver = &driver;
    Capture::callback = aCallback;
    Capture::hooked = true;

    return true;
}

template<typename F, typename... Args,
         typename R = typename F::return_type,
         typename = std::enable_if_t<std::is_same_v<typename F::type, detail::OriginalFunc<R, Args...>>>>
bool Before(detail::NoReturnCallback<Args...> aCallback)
{
    using Capture = detail::BeforeCapture<F, R, Args...>;

    if (Capture::hooked)
        return false;

    auto& driver = HookingDriver::GetDefault();
    const auto address = F::RelocAddr();

    if (!driver.HookAttach(address, &Capture::Handle, reinterpret_cast<void**>(&Capture::original)))
        return false;

    Capture::callback = aCallback;
    Capture::hooked = true;

    return true;
}

template<typename F, typename... Args,
         typename R = typename F::return_type,
         typename = std::enable_if_t<std::is_same_v<typename F::type, detail::OriginalFunc<R, Args...>>>>
bool After(detail::NoReturnCallback<Args...> aCallback)
{
    using Capture = detail::AfterCapture<F, R, Args...>;

    if (Capture::hooked)
        return false;

    auto& driver = HookingDriver::GetDefault();
    const auto address = F::RelocAddr();

    if (!driver.HookAttach(address, &Capture::Handle, reinterpret_cast<void**>(&Capture::original)))
        return false;

    Capture::callback = aCallback;
    Capture::hooked = true;

    return true;
}

template<typename F>
bool Drop()
{
    using Capture = detail::BaseCapture<F>;

    if (!Capture::hooked)
        return false;

    auto& driver = HookingDriver::GetDefault();
    const auto address = F::RelocAddr();

    if (!driver.HookDetach(address))
        return false;

    return true;
}
}

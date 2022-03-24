#pragma once

#include "HookingDriver.hpp"

#include <type_traits>

namespace Core::detail
{
template<typename R, typename... Args>
using OriginalFunc = R (*)(Args...);

template<typename R, typename... Args>
using StaticCallback = R (*)(Args...);

template<typename C, typename R, typename... Args>
using MemberCallback = R (C::*)(Args...);

template<typename... Args>
using NoReturnCallback = void (*)(Args...);

template<typename R, typename... Args>
using WrappedCallback = R (*)(OriginalFunc<R, Args...>, Args...);

template<typename F>
struct BaseCapture
{
    inline static bool hooked = false;
};

template<typename F, typename C, typename R, typename... Args>
struct MemberCapture : BaseCapture<F>
{
    inline static OriginalFunc<R, Args...> original;
    inline static MemberCallback<C, R, Args...> callback;
    inline static C* context;

    static R Handle(Args... aArgs)
    {
        return (context->*callback)(aArgs...);
    }
};

template<typename F, typename R, typename... Args>
struct WrappedCapture : BaseCapture<F>
{
    inline static OriginalFunc<R, Args...> original;
    inline static WrappedCallback<R, Args...> callback;

    static R Handle(Args... aArgs)
    {
        return callback(aArgs..., original);
    }
};

template<typename F, typename R, typename... Args>
struct WrappedOnceCapture : BaseCapture<F>
{
    inline static OriginalFunc<R, Args...> original;
    inline static WrappedCallback<R, Args...> callback;
    inline static Core::HookingDriver* driver;

    static R Handle(Args... aArgs)
    {
        R result = callback(aArgs..., original);

        if (driver->HookDetach(F::RelocAddr()))
        {
            original = nullptr;
            callback = nullptr;
            driver = nullptr;
        }

        return result;
    }
};

template<typename F, typename R, typename... Args>
struct BeforeCapture : BaseCapture<F>
{
    inline static OriginalFunc<R, Args...> original;
    inline static NoReturnCallback<Args...> callback;

    static R Handle(Args... aArgs)
    {
        callback(aArgs...);
        return original(aArgs...);
    }
};

template<typename F, typename R, typename... Args>
struct BeforeOnceCapture : BaseCapture<F>
{
    inline static OriginalFunc<R, Args...> original;
    inline static NoReturnCallback<Args...> callback;
    inline static Core::HookingDriver* driver;

    static R Handle(Args... aArgs)
    {
        callback(aArgs...);
        R result = original(aArgs...);

        if (driver->HookDetach(F::RelocAddr()))
        {
            original = nullptr;
            callback = nullptr;
            driver = nullptr;
        }

        return result;
    }
};

template<typename F, typename R, typename... Args>
struct AfterCapture : BaseCapture<F>
{
    inline static OriginalFunc<R, Args...> original;
    inline static NoReturnCallback<Args...> callback;

    static R Handle(Args... aArgs)
    {
        if constexpr (std::is_void_v<R>)
        {
            original(aArgs...);
            callback(aArgs...);
        }
        else
        {
            R result = original(aArgs...);
            callback(aArgs...);
            return result;
        }
    }
};
}

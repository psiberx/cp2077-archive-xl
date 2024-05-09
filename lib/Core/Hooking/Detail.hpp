#pragma once

#include "Core/Hooking/HookingDriver.hpp"
#include "Core/Stl.hpp"

namespace Core
{
enum class HookFlow
{
    Original,
    Before,
    After,
    Wrap,
};

enum class HookRun
{
    Default,
    Once,
};

template<auto TOffset>
struct HookResult
{
    HookResult()
        : success(false)
    {
    }

    HookResult(bool aSuccess)
        : success(aSuccess)
    {
    }

    constexpr operator bool() const noexcept
    {
        return success;
    }

    inline void OrThrow(const char* aMessage = nullptr) const
    {
        if (!success)
        {
            if (aMessage)
            {
                throw std::runtime_error(aMessage);
            }
            else
            {
                if constexpr (sizeof(TOffset) == sizeof(uintptr_t))
                {
                    throw std::runtime_error(std::format("Failed to hook at address {:#X}.", TOffset));
                }
                else
                {
                    throw std::runtime_error(std::format("Failed to hook address id {}.", TOffset));
                }
            }
        }
    }

    bool success;
};
}

namespace Core::Detail
{
template<typename R, typename... Args>
using OriginalFunc = R(*)(Args...);

template<typename... Args>
using ArgsObserverFunc = void(*)(Args...);

template<typename R, typename... Args>
using TailObserverFunc = void(*)(R&, Args...);

template<typename R>
using ResultObserverFunc = void(*)(R&);

using BlindObserver = void(*)();

template<typename R, typename... Args>
using FullWrapperFunc = R(*)(OriginalFunc<R, Args...>, Args...);

template<typename R, typename... Args>
using TailWrapperFunc = R(*)(R&, Args...);

template<typename R>
using ResultWrapperFunc = R(*)(R&);

// template<class C, typename R, typename... Args>
// using MemberFunc = R(C::*)(Args...);

template<typename TRaw, typename TCallback>
struct HookTraits : HookTraits<TRaw, decltype(&TCallback::operator())> {};

template<template<auto, typename> class TRaw, auto TOffset, typename TRawRet, typename... TRawArgs,
         typename TContext, typename TCallbackRet, typename... TCallbackArgs>
struct HookTraits<TRaw<TOffset, TRawRet(*)(TRawArgs...)>, TCallbackRet (TContext::*)(TCallbackArgs...)>
    : HookTraits<TRaw<TOffset, TRawRet(*)(TRawArgs...)>, TCallbackRet(*)(TCallbackArgs...)> {};

template<template<auto, typename> class TRaw, auto TOffset, typename TRawRet, typename... TRawArgs,
         typename TContext, typename TCallbackRet, typename... TCallbackArgs>
struct HookTraits<TRaw<TOffset, TRawRet(*)(TRawArgs...)>, TCallbackRet(TContext::*)(TCallbackArgs...) const>
    : HookTraits<TRaw<TOffset, TRawRet(*)(TRawArgs...)>, TCallbackRet(*)(TCallbackArgs...)> {};

template<template<auto, typename> class TRaw, auto TOffset, typename TRawRet, typename... TRawArgs,
         typename TCallbackRet, typename... TCallbackArgs>
requires std::is_void_v<TRawRet>
struct HookTraits<TRaw<TOffset, TRawRet(*)(TRawArgs...)>, TCallbackRet(*)(TCallbackArgs...)>
{
    using Callable = TCallbackRet(*)(TCallbackArgs...);

    static constexpr bool IsOriginal = std::is_same_v<Callable, OriginalFunc<TRawRet, TRawArgs...>>;

    static constexpr bool IsArgsObserver = std::is_same_v<Callable, ArgsObserverFunc<TRawArgs...>>;
    static constexpr bool IsTailObserver = false;
    static constexpr bool IsResultObserver = false;
    static constexpr bool IsBlindObserver = std::is_same_v<Callable, BlindObserver>;

    static constexpr bool IsFullWrapper = std::is_same_v<Callable, FullWrapperFunc<TRawRet, TRawArgs...>>;
    static constexpr bool IsTailWrapper = false;
    static constexpr bool IsResultWrapper = false;
};

template<template<auto, typename> class TRaw, auto TOffset, typename TRawRet, typename... TRawArgs,
         typename TCallbackRet, typename... TCallbackArgs>
requires (!std::is_void_v<TRawRet>)
struct HookTraits<TRaw<TOffset, TRawRet(*)(TRawArgs...)>, TCallbackRet(*)(TCallbackArgs...)>
{
    using Callable = TCallbackRet(*)(TCallbackArgs...);

    static constexpr bool IsOriginal = std::is_same_v<Callable, OriginalFunc<TRawRet, TRawArgs...>>;

    static constexpr bool IsArgsObserver = std::is_same_v<Callable, ArgsObserverFunc<TRawArgs...>>;
    static constexpr bool IsTailObserver = std::is_same_v<Callable, TailObserverFunc<TRawRet, TRawArgs...>>;
    static constexpr bool IsResultObserver = std::is_same_v<Callable, ResultObserverFunc<TRawRet>>;
    static constexpr bool IsBlindObserver = std::is_same_v<Callable, BlindObserver>;

    static constexpr bool IsFullWrapper = std::is_same_v<Callable, FullWrapperFunc<TRawRet, TRawArgs...>>;
    static constexpr bool IsTailWrapper = std::is_same_v<Callable, TailWrapperFunc<TRawRet, TRawArgs...>>;
    static constexpr bool IsResultWrapper = std::is_same_v<Callable, ResultWrapperFunc<TRawRet>>;
};

template<HookFlow TFlow, typename TRaw, typename TCallback>
struct HookFlowTraits
{
    using Traits = HookTraits<TRaw, TCallback>;

    static constexpr bool IsOriginalCompatible = Traits::IsOriginal;
    static constexpr bool IsBeforeCompatible = Traits::IsArgsObserver || Traits::IsBlindObserver;
    static constexpr bool IsAfterCompatible = Traits::IsArgsObserver || Traits::IsBlindObserver
                                              || Traits::IsTailObserver || Traits::IsResultObserver
                                              || Traits::IsTailWrapper || Traits::IsResultWrapper;
    static constexpr bool IsWrapCompatible = Traits::IsFullWrapper;

    static constexpr bool IsCompatible = (TFlow == HookFlow::Original && IsOriginalCompatible)
                                          || (TFlow == HookFlow::Before && IsBeforeCompatible)
                                          || (TFlow == HookFlow::After && IsAfterCompatible)
                                          || (TFlow == HookFlow::Wrap && IsWrapCompatible);

    static constexpr bool IsCompatibleMember = IsCompatible && std::is_member_pointer_v<TCallback>;
    static constexpr bool IsCompatibleNonMember = IsCompatible && !std::is_member_pointer_v<TCallback>;
};

template<typename TCallback>
class HookWrapper;

template<typename TRet, typename... TArgs>
class HookWrapper<TRet(*)(TArgs...)>
{
public:
    using Callback = TRet(*)(TArgs...);

    HookWrapper(Callback aCallback = nullptr)
        : m_callback(aCallback)
    {
    }

    inline TRet operator()(TArgs&&... aArgs) const
    {
        return m_callback(std::forward<TArgs>(aArgs)...);
    }

private:
    Callback m_callback;
};

template<typename TContext, typename TRet, typename... TArgs>
class HookWrapper<TRet(TContext::*)(TArgs...)>
{
public:
    using Callback = TRet(TContext::*)(TArgs...);
    using Context = TContext;

    HookWrapper()
        : m_callback(nullptr)
        , m_context(nullptr)
    {
    }

    HookWrapper(const Callback& aCallback, Context* aContext)
        : m_callback(aCallback)
        , m_context(aContext)
    {
    }

    template<class T>
    HookWrapper(const Callback& aCallback, T* aContext)
        : m_callback(aCallback)
        , m_context(static_cast<Context*>(aContext))
    {
    }

    inline TRet operator()(TArgs&&... aArgs) const
    {
        return (m_context->*m_callback)(std::forward<TArgs>(aArgs)...);
    }

private:
    Callback m_callback;
    Context* m_context;
};

template<typename TClosure, typename TRet, typename... TArgs>
class HookWrapper<TRet(TClosure::*)(TArgs...) const>
{
public:
    using Callback = TClosure;

    HookWrapper()
        : m_callback(nullptr)
    {
    }

    HookWrapper(Callback&& aCallback)
        : m_callback(MakeUnique<Callback>(std::move(aCallback)))
    {
    }

    inline TRet operator()(TArgs&&... aArgs) const
    {
        return (*m_callback)(std::forward<TArgs>(aArgs)...);
    }

private:
    UniquePtr<Callback> m_callback;
};

template<typename TCallback>
requires std::is_class_v<TCallback>
class HookWrapper<TCallback> : public HookWrapper<decltype(&TCallback::operator())>
{
    using Fallback = HookWrapper<decltype(&TCallback::operator())>;
    using Fallback::Fallback;
};

template<typename T>
concept CanOverrideAddress = requires(T, uintptr_t A)
{
    T::SetAddress(A);
    T::ResetAddress();
};

template<typename TRaw>
class HookInstance
{
public:
    using Raw = TRaw;
    using OriginalFunc = typename Raw::Callable;
    using HandleFunc = OriginalFunc;
    using DisposeFunc = void(*)();

    inline static bool Attach(HookingDriver& aDriver, HandleFunc aHandler, DisposeFunc aDisposer,
                              OriginalFunc* aOriginal)
    {
        if (s_attached)
            return false;

        s_driver = &aDriver;
        s_handler = aHandler;
        s_disposer = aDisposer;
        s_address = Raw::GetAddress();

        s_attached = s_driver->HookAttach(s_address,
            reinterpret_cast<void*>(s_handler),
            reinterpret_cast<void**>(aOriginal));
        s_original = *aOriginal;

        if (!s_attached)
        {
            if (s_disposer)
                s_disposer();

            return false;
        }

        if constexpr (CanOverrideAddress<TRaw>)
        {
            TRaw::SetAddress(reinterpret_cast<uintptr_t>(s_original));
        }

        return true;
    }

    inline static bool Detach(HandleFunc aHandler = nullptr)
    {
        if (!s_attached)
            return false;

        if (aHandler && s_handler != aHandler)
            return false;

        s_attached = !s_driver->HookDetach(s_address);

        if (s_attached)
            return false;

        if (s_disposer)
            s_disposer();

        s_handler = nullptr;
        s_disposer = nullptr;
        s_original = nullptr;
        s_driver = nullptr;
        s_address = 0;

        if constexpr (CanOverrideAddress<TRaw>)
        {
            TRaw::ResetAddress();
        }

        return true;
    }

    inline static bool IsAttached()
    {
        return s_attached;
    }

    inline static OriginalFunc GetOriginal()
    {
        return s_original;
    }

private:
    inline static HandleFunc s_handler = nullptr;
    inline static DisposeFunc s_disposer = nullptr;
    inline static OriginalFunc s_original = nullptr;
    inline static HookingDriver* s_driver = nullptr;
    inline static uintptr_t s_address = 0;
    inline static bool s_attached = false;
};

template<typename TRaw,
         typename TWrapper,
         HookFlow TFlow = HookFlow::Original,
         HookRun TRun = HookRun::Default>
class HookHandler;

template<template<auto, typename> class TRaw,
         auto TOffset,
         typename TRet,
         typename... TArgs,
         typename TWrapper,
         HookFlow TFlow,
         HookRun TRun>
class HookHandler<TRaw<TOffset, TRet(*)(TArgs...)>, TWrapper, TFlow, TRun>
{
public:
    using Raw = TRaw<TOffset, TRet(*)(TArgs...)>;
    using Original = typename Raw::Callable;
    using Wrapper = TWrapper;
    using Traits = HookTraits<Raw, typename Wrapper::Callback>;

    inline static TRet Handle(TArgs... aArgs)
    {
        if constexpr (std::is_void_v<TRet>)
        {
            if constexpr (TFlow == HookFlow::Original)
            {
                if constexpr (Traits::IsOriginal)
                {
                    s_callback(std::forward<TArgs>(aArgs)...);
                }
            }
            else if constexpr (TFlow == HookFlow::Before)
            {
                if constexpr (Traits::IsArgsObserver)
                {
                    s_callback(std::forward<TArgs>(aArgs)...);
                }
                else if constexpr (Traits::IsBlindObserver)
                {
                    s_callback();
                }

                s_original(std::forward<TArgs>(aArgs)...);
            }
            else if constexpr (TFlow == HookFlow::After)
            {
                s_original(std::forward<TArgs>(aArgs)...);

                if constexpr (Traits::IsArgsObserver)
                {
                    s_callback(std::forward<TArgs>(aArgs)...);
                }
                else if constexpr (Traits::IsBlindObserver)
                {
                    s_callback();
                }
            }
            else if constexpr (TFlow == HookFlow::Wrap)
            {
                if constexpr (Traits::IsFullWrapper)
                {
                    s_callback(std::forward<Original>(s_original), std::forward<TArgs>(aArgs)...);
                }
            }

            if constexpr (TRun == HookRun::Once)
            {
                Detach();
            }
        }
        else if constexpr (TFlow == HookFlow::Original)
        {
            if constexpr (Traits::IsOriginal)
            {
                TRet ret = s_callback(std::forward<TArgs>(aArgs)...);

                if constexpr (TRun == HookRun::Once)
                {
                    Detach();
                }

                return ret;
            }
        }
        else if constexpr (TFlow == HookFlow::Before)
        {
            if constexpr (Traits::IsArgsObserver)
            {
                s_callback(std::forward<TArgs>(aArgs)...);
            }
            else if constexpr (Traits::IsBlindObserver)
            {
                s_callback();
            }

            TRet ret = s_original(std::forward<TArgs>(aArgs)...);

            if constexpr (TRun == HookRun::Once)
            {
                Detach();
            }

            return ret;
        }
        else if constexpr (TFlow == HookFlow::After)
        {
            TRet ret = s_original(std::forward<TArgs>(aArgs)...);

            if constexpr (Traits::IsArgsObserver)
            {
                s_callback(std::forward<TArgs>(aArgs)...);
            }
            else if constexpr (Traits::IsTailObserver)
            {
                s_callback(ret, std::forward<TArgs>(aArgs)...);
            }
            else if constexpr (Traits::IsResultObserver)
            {
                s_callback(ret);
            }
            else if constexpr (Traits::IsBlindObserver)
            {
                s_callback();
            }
            else if constexpr (Traits::IsTailWrapper)
            {
                ret = s_callback(ret, std::forward<TArgs>(aArgs)...);
            }
            else if constexpr (Traits::IsResultWrapper)
            {
                ret = s_callback(ret);
            }

            if constexpr (TRun == HookRun::Once)
            {
                Detach();
            }

            return ret;
        }
        else if constexpr (TFlow == HookFlow::Wrap)
        {
            if constexpr (Traits::IsFullWrapper)
            {
                TRet ret = s_callback(std::forward<Original>(s_original), std::forward<TArgs>(aArgs)...);

                if constexpr (TRun == HookRun::Once)
                {
                    Detach();
                }

                return ret;
            }
        }
    }

    inline static HookResult<TOffset> Attach(HookingDriver& aDriver, TWrapper aCallback, Original* aOriginal = nullptr)
    {
        using Instance = HookInstance<Raw>;

        s_callback = std::move(aCallback);

        if (!Instance::Attach(aDriver, &Handle, &Dispose, &s_original))
            return false;

        if (aOriginal)
            *aOriginal = s_original;

        return true;
    }

    inline static bool Detach()
    {
        using Instance = HookInstance<Raw>;
        return Instance::Detach(&Handle);
    }

    inline static void Dispose()
    {
        s_callback = {};
    }

private:
    static inline Wrapper s_callback;
    static inline Original s_original;
};
}

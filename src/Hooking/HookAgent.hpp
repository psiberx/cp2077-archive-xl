#pragma once

class HookAgent
{
private:
    template<typename R, typename ...Args>
    using OriginalFunc = R (*)(Args...);

    template<typename R, typename ...Args>
    using StaticCallback = OriginalFunc<R, Args...>;

    template<typename C, typename R, typename ...Args>
    using MemberCallback = R (C::*)(Args...);

    template<typename C, typename R, typename ...Args>
    struct MemberCapture
    {
        inline static OriginalFunc<R, Args...> original;
        inline static MemberCallback<C, R, Args...> callback;
        inline static C* context;

        static R Handle(Args... aArgs)
        {
            return (context->*callback)(aArgs...);
        }
    };

public:
    template<typename C, typename R, typename ...Args>
    bool Hook(const uintptr_t aOffset, C* aContext, MemberCallback<C, R, Args...> aCallback, OriginalFunc<R, Args...>* aOriginal = nullptr)
    {
        using Capture = MemberCapture<C, R, Args...>;

        if (!Capture::original)
        {
            const auto pAddress = RED4ext::RelocBase::GetImageBase() + aOffset;

            if (MH_CreateHook(reinterpret_cast<void*>(pAddress), &Capture::Handle, reinterpret_cast<void**>(&Capture::original)) != MH_OK)
                return false;

            if (MH_EnableHook(reinterpret_cast<void*>(pAddress)) != MH_OK)
                return false;
        }

        if (aOriginal)
            *aOriginal = Capture::original;

        Capture::callback = aCallback;
        Capture::context = aContext;

        return true;
    }

    template<typename R, typename ...Args>
    bool Hook(const uintptr_t aOffset, StaticCallback<R, Args...> aCallback, OriginalFunc<R, Args...>* aOriginal = nullptr)
    {
        const auto pAddress = RED4ext::RelocBase::GetImageBase() + aOffset;
        OriginalFunc<R, Args...>* pOriginal;

        if (MH_CreateHook(reinterpret_cast<void*>(pAddress), &aCallback, reinterpret_cast<void**>(&pOriginal)) != MH_OK)
            return false;

        if (MH_EnableHook(reinterpret_cast<void*>(pAddress)) != MH_OK)
            return false;

        if (aOriginal)
            *aOriginal = pOriginal;

        return true;
    }
};

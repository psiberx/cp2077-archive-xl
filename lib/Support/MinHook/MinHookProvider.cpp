#include "MinHookProvider.hpp"

#include <MinHook.h>

Support::MinHookProvider::MinHookProvider()
{
    MH_Initialize();

    SetDefault(*this);
}

Support::MinHookProvider::~MinHookProvider()
{
    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();
}

bool Support::MinHookProvider::HookAttach(uintptr_t aAddress, void* aCallback)
{
    return HookAttach(aAddress, aCallback, nullptr);
}

bool Support::MinHookProvider::HookAttach(uintptr_t aAddress, void* aCallback, void** aOriginal)
{
    if (MH_CreateHook(reinterpret_cast<void*>(aAddress), aCallback, aOriginal) != MH_OK)
        return false;

    if (MH_EnableHook(reinterpret_cast<void*>(aAddress)) != MH_OK)
    {
        MH_RemoveHook(reinterpret_cast<void*>(aAddress));
        return false;
    }

    return true;
}

bool Support::MinHookProvider::HookDetach(uintptr_t aAddress)
{
    if (MH_DisableHook(reinterpret_cast<void*>(aAddress)) != MH_OK)
        return false;

    if (MH_RemoveHook(reinterpret_cast<void*>(aAddress)) != MH_OK)
        return false;

    return true;
}

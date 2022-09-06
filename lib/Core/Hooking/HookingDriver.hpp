#pragma once

namespace Core
{
class HookingDriver
{
public:
    virtual bool HookAttach(uintptr_t aAddress, void* aCallback) = 0;
    virtual bool HookAttach(uintptr_t aAddress, void* aCallback, void** aOriginal) = 0;
    virtual bool HookDetach(uintptr_t aAddress) = 0;

    static void SetDefault(HookingDriver& aDriver);
    static HookingDriver& GetDefault();
};
}

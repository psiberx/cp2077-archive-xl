#pragma once

#include "Core/Foundation/Feature.hpp"
#include "Core/Hooking/HookingDriver.hpp"

namespace Support
{
class MinHookProvider
    : public Core::Feature
    , public Core::HookingDriver
{
public:
    MinHookProvider();
    ~MinHookProvider() override;

    bool HookAttach(uintptr_t aAddress, void* aCallback) override;
    bool HookAttach(uintptr_t aAddress, void* aCallback, void** aOriginal) override;
    bool HookDetach(uintptr_t aAddress) override;
};
}

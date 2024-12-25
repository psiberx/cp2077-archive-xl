#pragma once

#include "Core/Foundation/Feature.hpp"
#include "Core/Hooking/HookingAgent.hpp"
#include "Core/Logging/LoggingAgent.hpp"

namespace App
{
class PhotoModeNpcPatch
    : public Core::Feature
    , public Core::HookingAgent
    , public Core::LoggingAgent
{
protected:
    void OnBootstrap() override;
};
}

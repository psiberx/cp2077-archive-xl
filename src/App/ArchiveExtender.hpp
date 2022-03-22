#pragma once

#include "stdafx.hpp"
#include "Core/Foundation/Feature.hpp"
#include "Core/Logging/LoggingAgent.hpp"
#include "Engine/Scripting/RTTIClass.hpp"

namespace App
{
class ArchiveExtender
    : public Core::Feature
    , public Core::LoggingAgent
    , public Engine::RTTIClass<ArchiveExtender>
{
public:
    static void Reload();
    static RED4ext::CString Version();

private:
    void OnBootstrap() override;
    void OnShutdown() override;

    friend Descriptor;
    static void OnRegister(Descriptor* aType);
    static void OnDescribe(Descriptor* aType, RED4ext::CRTTISystem* aRtti);
};
}

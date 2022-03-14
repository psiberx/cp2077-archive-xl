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
protected:
    void OnBootstrap() override;
    void OnShutdown() override;

private:
    friend Descriptor;
    static void OnRegister(Descriptor* aType);
    static void OnBuild(Descriptor* aType, RED4ext::CRTTISystem* aRtti);
    static void Reload();
};
}

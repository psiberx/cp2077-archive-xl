#pragma once

#include "App/Extensions/ModuleLoader.hpp"
#include "Core/Foundation/Feature.hpp"
#include "Core/Hooking/HookingAgent.hpp"
#include "Core/Logging/LoggingAgent.hpp"

namespace App
{
class ExtensionService
    : public Core::Feature
    , public Core::HookingAgent
    , public Core::LoggingAgent
{
public:
    void Configure();

protected:
    void OnBootstrap() override;
    void OnShutdown() override;

    Core::UniquePtr<ModuleLoader> m_loader;
    std::mutex m_reloadMutex;
};
}

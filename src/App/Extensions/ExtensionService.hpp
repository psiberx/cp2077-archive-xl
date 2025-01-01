#pragma once

#include "App/Extensions/ExtensionLoader.hpp"
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
    explicit ExtensionService(std::filesystem::path aBundleDir = {});

    void Configure();

protected:
    void OnBootstrap() override;
    void OnShutdown() override;

    Core::UniquePtr<ExtensionLoader> m_loader;
    std::filesystem::path m_bundleDir;
    std::mutex m_reloadMutex;
};
}

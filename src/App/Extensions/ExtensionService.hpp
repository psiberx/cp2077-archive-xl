#pragma once

#include "App/Module/ModuleLoader.hpp"
#include "Core/Foundation/Feature.hpp"
#include "Core/Logging/LoggingAgent.hpp"

namespace App
{
class ExtensionService
    : public Core::Feature
    , public Core::LoggingAgent
{
public:
    ExtensionService(std::filesystem::path  mArchiveModDir);

    void Configure();

protected:
    void OnBootstrap() override;
    void OnShutdown() override;

    std::filesystem::path m_archiveModDir;
    Core::UniquePtr<ModuleLoader> m_loader;
};
}

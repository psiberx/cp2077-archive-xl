#include "ExtensionService.hpp"
#include "App/Extensions/FactoryIndex/Module.hpp"
#include "App/Extensions/InkSpawner/Module.hpp"
#include "App/Extensions/Localization/Module.hpp"
#include "App/Extensions/PartsOverrides/Module.hpp"
#include "App/Extensions/VisualTags/Module.hpp"

App::ExtensionService::ExtensionService(std::filesystem::path aModDir)
    : m_archiveModDir(std::move(aModDir))
{
}

void App::ExtensionService::OnBootstrap()
{
    m_loader = Core::MakeUnique<ModuleLoader>(m_archiveModDir, L".xl");

    m_loader->Add<FactoryIndexModule>();
    m_loader->Add<LocalizationModule>();
    m_loader->Add<PartsOverridesModule>();
    m_loader->Add<VisualTagsModule>();
    m_loader->Add<InkSpawnerModule>();

    m_loader->Configure();
    m_loader->Load();
}

void App::ExtensionService::OnShutdown()
{
    m_loader->Unload();
    m_loader = nullptr;
}

void App::ExtensionService::Configure()
{
    m_loader->Configure();
}

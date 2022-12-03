#include "ExtensionService.hpp"
#include "App/Extensions/AppearanceSwap/Module.hpp"
#include "App/Extensions/AttachmentSlots/Module.hpp"
#include "App/Extensions/FactoryIndex/Module.hpp"
#include "App/Extensions/InkSpawner/Module.hpp"
#include "App/Extensions/Localization/Module.hpp"
#include "App/Extensions/PartsOverrides/Module.hpp"
#include "App/Extensions/VisualTags/Module.hpp"
#include "Red/GameApplication.hpp"

void App::ExtensionService::OnBootstrap()
{
    m_loader = Core::MakeUnique<ModuleLoader>("", L".xl");

    m_loader->Add<AppearanceSwapModule>();
    m_loader->Add<AttachmentSlotsModule>();
    m_loader->Add<FactoryIndexModule>();
    m_loader->Add<LocalizationModule>();
    m_loader->Add<PartsOverridesModule>();
    m_loader->Add<VisualTagsModule>();
    m_loader->Add<InkSpawnerModule>();

    HookOnceAfter<Raw::GameApplication::InitResourceDepot>([&]() {
        m_loader->Configure();
        m_loader->Load();
    });
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

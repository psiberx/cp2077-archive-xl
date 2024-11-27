#include "ExtensionService.hpp"
#include "App/Extensions/Animation/Module.hpp"
#include "App/Extensions/Attachment/Module.hpp"
#include "App/Extensions/Customization/Module.hpp"
#include "App/Extensions/FactoryIndex/Module.hpp"
#include "App/Extensions/Garment/Module.hpp"
#include "App/Extensions/InkSpawner/Module.hpp"
#include "App/Extensions/Journal/Module.hpp"
#include "App/Extensions/Localization/Module.hpp"
#include "App/Extensions/Mesh/Module.hpp"
#include "App/Extensions/PuppetState/Module.hpp"
#include "App/Extensions/QuestPhase/Module.hpp"
#include "App/Extensions/ResourceLink/Module.hpp"
#include "App/Extensions/ResourceMeta/Module.hpp"
#include "App/Extensions/ResourcePatch/Module.hpp"
#include "App/Extensions/Transmog/Module.hpp"
#include "App/Extensions/WorldStreaming/Module.hpp"
#include "Red/GameApplication.hpp"
#include "Red/GameEngine.hpp"
#include "Red/ResourceLoader.hpp"

App::ExtensionService::ExtensionService(std::filesystem::path aBundlePath)
    : m_bundlePath(std::move(aBundlePath))
{
}

void App::ExtensionService::OnBootstrap()
{
    m_loader = Core::MakeUnique<ModuleLoader>(m_bundlePath, L".xl");

    m_loader->Add<ResourceMetaModule>();
    m_loader->Add<ResourceLinkModule>();
    m_loader->Add<ResourcePatchModule>();
    m_loader->Add<MeshModule>();
    m_loader->Add<FactoryIndexModule>();
    m_loader->Add<LocalizationModule>();
    m_loader->Add<JournalModule>();
    m_loader->Add<AnimationModule>();
    m_loader->Add<TransmogModule>();
    m_loader->Add<AttachmentModule>();
    m_loader->Add<CustomizationModule>();
    m_loader->Add<GarmentModule>();
    m_loader->Add<PuppetStateModule>();
    m_loader->Add<QuestPhaseModule>();
    m_loader->Add<WorldStreamingModule>();
    m_loader->Add<InkSpawnerModule>();

    HookOnceAfter<Raw::GameApplication::InitResourceDepot>([&]() {
        m_loader->Configure();
        m_loader->Load();
    });

    HookOnceAfter<Raw::CBaseEngine::LoadGatheredResources>([&]() {
        m_loader->PostLoad();
    });
}

void App::ExtensionService::OnShutdown()
{
    m_loader->Unload();
    m_loader = nullptr;
}

void App::ExtensionService::Configure()
{
    std::unique_lock _(m_reloadMutex);

    if (!IsHooked<Raw::ResourceLoader::OnUpdate>())
    {
        HookAfter<Raw::ResourceLoader::OnUpdate>([&]() {
            std::unique_lock _(m_reloadMutex);

            m_loader->Configure();
            m_loader->Reload();

            Unhook<Raw::ResourceLoader::OnUpdate>();
        });
    }
}

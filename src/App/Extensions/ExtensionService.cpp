#include "ExtensionService.hpp"
#include "App/Extensions/Animation/Extension.hpp"
#include "App/Extensions/Attachment/Extension.hpp"
#include "App/Extensions/Customization/Extension.hpp"
#include "App/Extensions/FactoryIndex/Extension.hpp"
#include "App/Extensions/Garment/Extension.hpp"
#include "App/Extensions/InkSpawner/Extension.hpp"
#include "App/Extensions/Journal/Extension.hpp"
#include "App/Extensions/Localization/Extension.hpp"
#include "App/Extensions/Mesh/Extension.hpp"
#include "App/Extensions/PuppetState/Extension.hpp"
#include "App/Extensions/QuestPhase/Extension.hpp"
#include "App/Extensions/ResourceLink/Extension.hpp"
#include "App/Extensions/ResourceMeta/Extension.hpp"
#include "App/Extensions/ResourcePatch/Extension.hpp"
#include "App/Extensions/Transmog/Extension.hpp"
#include "App/Extensions/WorldStreaming/Extension.hpp"
#include "Red/GameApplication.hpp"
#include "Red/GameEngine.hpp"
#include "Red/ResourceLoader.hpp"
#include "Red/TweakDB.hpp"

App::ExtensionService::ExtensionService(std::filesystem::path aBundlePath)
    : m_bundlePath(std::move(aBundlePath))
{
}

void App::ExtensionService::OnBootstrap()
{
    m_loader = Core::MakeUnique<ExtensionLoader>(m_bundlePath, L".xl");

    m_loader->Add<ResourceMetaExtension>();
    m_loader->Add<ResourceLinkExtension>();
    m_loader->Add<ResourcePatchExtension>();
    m_loader->Add<MeshExtension>();
    m_loader->Add<FactoryIndexExtension>();
    m_loader->Add<LocalizationExtension>();
    m_loader->Add<JournalExtension>();
    m_loader->Add<AnimationExtension>();
    m_loader->Add<TransmogExtension>();
    m_loader->Add<AttachmentExtension>();
    m_loader->Add<CustomizationExtension>();
    m_loader->Add<GarmentExtension>();
    m_loader->Add<PuppetStateExtension>();
    m_loader->Add<QuestPhaseExtension>();
    m_loader->Add<WorldStreamingExtension>();
    m_loader->Add<InkSpawnerExtension>();

    HookOnceAfter<Raw::GameApplication::InitResourceDepot>([&]() {
        m_loader->Configure();
        m_loader->Load();
    });

    HookOnceAfter<Raw::CBaseEngine::LoadGatheredResources>([&]() {
        m_loader->PostLoad();
    });

    HookAfter<Raw::LoadTweakDB>([&]() {
        m_loader->ApplyTweaks();
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

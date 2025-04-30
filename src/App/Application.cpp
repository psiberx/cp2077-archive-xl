#include "Application.hpp"
#include "App/Archives/ArchiveService.hpp"
#include "App/Environment.hpp"
#include "App/Extensions/ExtensionService.hpp"
#include "App/Migration.hpp"
#include "App/Patches/EntitySpawnerPatch.hpp"
#include "App/Patches/WorldWidgetLimitPatch.hpp"
#include "App/Project.hpp"
#include "App/Shared/ResourcePathRegistry.hpp"
#include "Core/Foundation/RuntimeProvider.hpp"
#include "Support/MinHook/MinHookProvider.hpp"
#include "Support/RED4ext/RED4extProvider.hpp"
#include "Support/RedLib/RedLibProvider.hpp"
#include "Support/Spdlog/SpdlogProvider.hpp"

App::Application::Application(HMODULE aHandle, const RED4ext::Sdk* aSdk)
{
    Register<Core::RuntimeProvider>(aHandle)
        ->SetBaseImagePathDepth(2);

    Register<Support::MinHookProvider>();
    Register<Support::SpdlogProvider>()
        ->AppendTimestampToLogName()
        ->CreateRecentLogSymlink();
    Register<Support::RED4extProvider>(aHandle, aSdk)
        ->EnableAddressLibrary()
        ->RegisterScripts(Env::ScriptsDir());
    Register<Support::RedLibProvider>();

    Register<App::ResourcePathRegistry>();
    Register<App::ArchiveService>(Env::GameDir(), Env::BundleDir());
    Register<App::ExtensionService>(Env::BundleDir());
    Register<App::EntitySpawnerPatch>();
    Register<App::WorldWidgetLimitPatch>();
}

void App::Application::OnStarting()
{
    LogInfo("{} {} is starting...", Project::Name, Project::Version.to_string());

    Migration::CleanUp(Env::LegacyBundleDir());
    Migration::CleanUp(Env::LegacyScriptsDir());
}

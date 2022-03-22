#include "Application.hpp"
#include "ArchiveExtender.hpp"
#include "Core/Providers/RuntimeProvider.hpp"
#include "Vendor/MinHook/MinHookProvider.hpp"
#include "Vendor/RED4ext/RED4extProvider.hpp"
#include "Vendor/Spdlog/SpdlogProvider.hpp"

App::Application::Application(HMODULE aHandle, const RED4ext::Sdk* aSdk)
{
    Register<Core::RuntimeProvider>({ .handle = aHandle, .exePathDepth = 2 });

    Register<Vendor::MinHookProvider>();
    Register<Vendor::SpdlogProvider>();

    if (aSdk)
        Register<Vendor::RED4extProvider>({ .plugin = aHandle, .sdk = aSdk, .logging = false });

    Register<App::ArchiveExtender>();
}

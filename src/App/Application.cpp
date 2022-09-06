#include "Application.hpp"
#include "App/Config.hpp"
#include "App/Facade.hpp"
#include "App/Extensions/ExtensionService.hpp"
#include "Core/Runtime/RuntimeProvider.hpp"
#include "Engine/Scripting/RTTIProvider.hpp"
#include "Vendor/MinHook/MinHookProvider.hpp"
#include "Vendor/RED4ext/RED4extProvider.hpp"
#include "Vendor/Spdlog/SpdlogProvider.hpp"

App::Application::Application(HMODULE aHandle, const RED4ext::Sdk* aSdk)
{
    Register<Core::RuntimeProvider>(aHandle)->SetBaseImagePathDepth(2);
    Register<Vendor::MinHookProvider>();
    Register<Vendor::SpdlogProvider>();
    Register<Engine::RTTIProvider>();
    Register<App::ExtensionService>(Config::GetModArchiveDir());
}

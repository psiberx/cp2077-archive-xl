#include "Application.hpp"
#include "App/Facade.hpp"
#include "App/Extensions/ExtensionService.hpp"
#include "Core/Foundation/RuntimeProvider.hpp"
#include "Red/Foundation/RttiProvider.hpp"
#include "Vendor/MinHook/MinHookProvider.hpp"
#include "Vendor/RED4ext/RED4extProvider.hpp"
#include "Vendor/Spdlog/SpdlogProvider.hpp"

App::Application::Application(HMODULE aHandle, const RED4ext::Sdk* aSdk)
{
    Register<Core::RuntimeProvider>(aHandle)->SetBaseImagePathDepth(2);
    Register<Vendor::MinHookProvider>();
    Register<Vendor::SpdlogProvider>();
    Register<Red::RttiProvider>();
    Register<App::ExtensionService>();
}

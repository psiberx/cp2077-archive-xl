#include "ArchiveExtender.hpp"
#include "App/Configuration.hpp"
#include "App/Common/ModuleLoader.hpp"
#include "App/FactoryIndex/Module.hpp"
#include "App/Localization/Module.hpp"

namespace
{
Core::UniquePtr<App::ModuleLoader> s_loader;
}

void App::ArchiveExtender::OnBootstrap()
{
    RegisterRTTI();

    s_loader = Core::MakeUnique<ModuleLoader>(Configuration::GetModArchiveDir(), L".xl");

    s_loader->Add<FactoryIndexModule>();
    s_loader->Add<LocalizationModule>();

    s_loader->Configure();
    s_loader->Load();
}

void App::ArchiveExtender::OnShutdown()
{
    s_loader->Unload();
    s_loader = nullptr;
}

void App::ArchiveExtender::Reload()
{
    s_loader->Configure();
}

void App::ArchiveExtender::OnRegister(Descriptor* aType)
{
    aType->SetName("ArchiveXL");
}

void App::ArchiveExtender::OnBuild(Descriptor* aType, RED4ext::CRTTISystem* aRtti)
{
    aType->AddFunction<&Reload>("Reload");
}

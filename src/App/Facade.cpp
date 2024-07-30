#include "Facade.hpp"
#include "App/Archives/ArchiveService.hpp"
#include "App/Extensions/ExtensionService.hpp"
#include "App/Extensions/GarmentOverride/Module.hpp"
#include "App/Extensions/PuppetState/Module.hpp"
#include "Core/Facades/Container.hpp"

bool App::Facade::RegisterDir(Red::CString& aPath)
{
    return Core::Resolve<ArchiveService>()->RegisterDirectory(aPath.c_str());
}

bool App::Facade::RegisterArchive(Red::CString& aPath)
{
    return Core::Resolve<ArchiveService>()->RegisterArchive(aPath.c_str());
}

Red::CName App::Facade::GetBodyType(const Red::WeakHandle<Red::GameObject>& aPuppet)
{
    return PuppetStateModule::GetBodyType(aPuppet);
}

void App::Facade::EnableGarmentOffsets()
{
    GarmentOverrideModule::EnableGarmentOffsets();
}

void App::Facade::DisableGarmentOffsets()
{
    GarmentOverrideModule::DisableGarmentOffsets();
}

void App::Facade::Reload()
{
    Core::Resolve<ExtensionService>()->Configure();
}

bool App::Facade::Require(Red::CString& aVersion)
{
    const auto requirement = semver::from_string_noexcept(aVersion.c_str());
    return requirement.has_value() && Project::Version >= requirement.value();
}

Red::CString App::Facade::GetVersion()
{
    return Project::Version.to_string().c_str();
}

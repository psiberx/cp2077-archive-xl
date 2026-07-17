#include "Facade.hpp"
#include "App/Archives/ArchiveService.hpp"
#include "App/Extensions/ExtensionService.hpp"
#include "App/Extensions/Garment/Extension.hpp"
#include "App/Extensions/PuppetState/Extension.hpp"
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
    return PuppetStateExtension::GetBodyType(aPuppet);
}

Red::CString App::Facade::ResolveDynamicAppearanceString(const Red::WeakHandle<Red::GameObject>& aPuppet,
                                                         const Red::CString& aAppearance, const Red::CString& aString)
{
    auto controller = GarmentExtension::GetDynamicAppearanceController();
    auto appearance = DynamicAppearanceName(aAppearance);

    return controller->ResolveString(aPuppet.Lock(), appearance.parts, aString);
}

Red::CString App::Facade::ResolveDynamicAppearancePath(const Red::WeakHandle<Red::GameObject>& aPuppet,
                                                         const Red::CString& aAppearance, const Red::CString& aPath)
{
    auto controller = GarmentExtension::GetDynamicAppearanceController();
    auto pathRegistry = Core::Resolve<ResourcePathRegistry>();

    auto appearance = DynamicAppearanceName(aAppearance);
    auto path = pathRegistry->RegisterPath(aPath.c_str());

    path = controller->ResolvePath(aPuppet.Lock(), appearance.parts, path);

    return pathRegistry->ResolvePath(path);
}

void App::Facade::EnableGarmentOffsets()
{
    GarmentExtension::EnableGarmentOffsets();
}

void App::Facade::DisableGarmentOffsets()
{
    GarmentExtension::DisableGarmentOffsets();
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

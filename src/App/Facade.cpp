#include "Facade.hpp"
#include "App/Project.hpp"
#include "App/Extensions/ExtensionService.hpp"
#include "Core/Container/Container.hpp"

void App::Facade::Reload()
{
    Core::Resolve<ExtensionService>()->Configure();
}

RED4ext::CString App::Facade::GetVersion()
{
    return Project::Version.to_string().c_str();
}

void App::Facade::OnRegister(Descriptor* aType)
{
    aType->SetName(Project::Name);
    aType->SetFlags({ .isAbstract = true });
}

void App::Facade::OnDescribe(Descriptor* aType, RED4ext::CRTTISystem*)
{
    aType->AddFunction<&Reload>("Reload");
    aType->AddFunction<&GetVersion>("Version");
}

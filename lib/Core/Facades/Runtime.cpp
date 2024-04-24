#include "Runtime.hpp"
#include "Core/Stl.hpp"

namespace
{
Core::UniquePtr<Core::HostImage> s_host;
Core::UniquePtr<Core::ModuleImage> s_module;
}

void Core::Runtime::Initialize(const Core::HostImage& aHost, const Core::ModuleImage& aModule)
{
    s_host = Core::MakeUnique<Core::HostImage>(aHost);
    s_module = Core::MakeUnique<Core::ModuleImage>(aModule);
}

Core::HostImage* Core::Runtime::GetHost()
{
    return s_host.get();
}

Core::ModuleImage* Core::Runtime::GetModule()
{
    return s_module.get();
}

uintptr_t Core::Runtime::GetImageBase()
{
    assert(s_host);
    return s_host->GetBase();
}

std::filesystem::path Core::Runtime::GetImagePath()
{
    assert(s_host);
    return s_host->GetPath();
}

std::filesystem::path Core::Runtime::GetRootDir()
{
    assert(s_host);
    return s_host->GetRootDir();
}

std::filesystem::path Core::Runtime::GetModulePath()
{
    assert(s_module);
    return s_module->GetPath();
}

std::filesystem::path Core::Runtime::GetModuleDir()
{
    assert(s_module);
    return s_module->GetDir();
}

std::string Core::Runtime::GetModuleName()
{
    assert(s_module);
    return s_module->GetName();
}

bool Core::Runtime::IsASI()
{
    assert(s_module);
    return s_module->IsASI();
}

bool Core::Runtime::IsASI(HMODULE aHandle)
{
    return Core::ModuleImage(aHandle).IsASI();
}

bool Core::Runtime::IsEXE(std::wstring_view aName)
{
    if (s_host)
        return s_host->GetPath().filename() == aName;

    return Core::HostImage().GetPath().filename() == aName;
}

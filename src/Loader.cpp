#include "stdafx.hpp"
#include "Loader.hpp"
#include "Extensions/Localization/LocalizationExt.hpp"
#include "Extensions/FactoryIndex/FactoryIndexExt.hpp"

namespace
{
std::unique_ptr<AXL::Loader> g_loader;
}

void AXL::Loader::Construct()
{
    g_loader.reset(new Loader());
}

void AXL::Loader::Destruct()
{
    g_loader.reset(nullptr);
}

AXL::Loader* AXL::Loader::Get()
{
    return g_loader.get();
}

AXL::Loader::Loader()
{
    MH_Initialize();

    m_extensions.emplace_back(new LocalizationExtension());
    m_extensions.emplace_back(new FactoryIndexExtension());

    Configure();
    Attach();

    MH_EnableHook(MH_ALL_HOOKS);
}

AXL::Loader::~Loader()
{
    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();

    Detach();

    m_extensions.clear();
}

void AXL::Loader::Configure()
{
    auto archiveDir = m_env.GetArchiveModDir();

    if (std::filesystem::exists(archiveDir))
    {
        auto archiveDirIter = std::filesystem::recursive_directory_iterator(
                archiveDir, std::filesystem::directory_options::follow_directory_symlink);

        for (const auto& entry : archiveDirIter)
        {
            if (entry.is_regular_file() && entry.path().extension() == ConfigExtension)
            {
                const auto name = entry.path().filename().string();
                const auto config = YAML::LoadFile(entry.path().string());

                for (const auto& extension : m_extensions)
                    extension->Configure(name, config);
            }
        }
    }
}

void AXL::Loader::Attach()
{
    for (const auto& extension : m_extensions)
        extension->Attach();
}

void AXL::Loader::Detach()
{
    for (const auto& extension : m_extensions)
        extension->Detach();
}

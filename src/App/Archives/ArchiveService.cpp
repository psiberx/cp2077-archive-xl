#include "ArchiveService.hpp"
#include "Core/Facades/Runtime.hpp"

App::ArchiveService::ArchiveService(std::filesystem::path aGameDir, std::filesystem::path aBundleDir)
    : m_gameDir(std::move(aGameDir))
    , m_bundleDir(std::move(aBundleDir))
    , m_loaded(false)
{
    if (!m_bundleDir.empty())
    {
        RegisterDirectory(m_bundleDir);
    }
}

void App::ArchiveService::OnBootstrap()
{
    HookAfter<Raw::ResourceDepot::InitializeArchives>(&ArchiveService::OnInitializeArchives).OrThrow();
}

void App::ArchiveService::OnShutdown()
{
    Unhook<Raw::ResourceDepot::InitializeArchives>();
}

void App::ArchiveService::OnInitializeArchives(Red::ResourceDepot* aDepot)
{
    Red::RegisterPendingTypes();

    m_loaded = true;

    LogInfo("Loading extra archives...");

    if (m_dirs.empty() && m_archives.empty())
        return;

    Core::Vector<std::filesystem::path> loadedArchives;
    Red::DynArray<Red::ResourcePath> loadedResources;

    for (const auto& archiveDir : m_dirs)
    {
        std::error_code error;
        auto dirIt = std::filesystem::directory_iterator(archiveDir, error);

        if (error)
        {
            LogError("Can't load archive directory \"{}\": {}",
                     std::filesystem::relative(archiveDir, m_gameDir).string(),
                     error.message());
            continue;
        }

        Red::DynArray<Red::CString> archivePaths;

        for (const auto& entry : dirIt)
        {
            if (entry.is_regular_file() && entry.path().extension() == L".archive")
            {
                archivePaths.PushBack(entry.path().string());

                if (archiveDir != m_bundleDir)
                {
                    loadedArchives.push_back(entry.path());
                }
            }
        }

        auto& group = ResolveArchiveGroup(aDepot, archiveDir.string());

        if (archivePaths.size > 0)
        {
            Raw::ResourceDepot::LoadArchives(nullptr, group, archivePaths, loadedResources, false);
        }
    }

    if (!m_archives.empty())
    {
        Red::DynArray<Red::CString> archivePaths;

        for (const auto& archivePath : m_archives)
        {
            archivePaths.PushBack(archivePath.string());
            loadedArchives.push_back(archivePath);
        }

        auto& group = ResolveArchiveGroup(aDepot, "");
        Raw::ResourceDepot::LoadArchives(nullptr, group, archivePaths, loadedResources, false);
    }

    for (const auto& archivePath : loadedArchives)
    {
        LogInfo("Archive \"{}\" loaded.", std::filesystem::relative(archivePath, m_gameDir).string());
    }
}

Red::ArchiveGroup& App::ArchiveService::ResolveArchiveGroup(Red::ResourceDepot* aDepot, const Red::CString& aBasePath)
{
    auto existingGroup = std::find_if(aDepot->groups.begin(), aDepot->groups.end(),
                                     [&aBasePath](const Red::ArchiveGroup& aGroup) {
                                         return aGroup.basePath == aBasePath;
                                     });

    if (existingGroup != aDepot->groups.end())
    {
        return *existingGroup;
    }

    auto firstNonModGroup = std::find_if(aDepot->groups.begin(), aDepot->groups.end(),
                                         [](const Red::ArchiveGroup& aGroup) {
                                             return aGroup.scope != Red::ArchiveScope::Mod;
                                         });
    auto firstNonModGroupIndex = firstNonModGroup - aDepot->groups.begin();

    aDepot->groups.Emplace(firstNonModGroup);

    auto& group = aDepot->groups[firstNonModGroupIndex];
    group.basePath = aBasePath;
    group.scope = Red::ArchiveScope::Mod;

    return group;
}

bool App::ArchiveService::RegisterArchive(std::filesystem::path aPath)
{
    std::error_code error;

    if (aPath.is_relative())
    {
        aPath = m_gameDir / aPath;
    }

    if (!std::filesystem::exists(aPath, error) || !std::filesystem::is_regular_file(aPath, error))
    {
        LogError("Can't register archive \"{}\": path doesn't exist.",
                 std::filesystem::relative(aPath, m_gameDir).string());
        return false;
    }

    if (m_loaded)
    {
        LogError("Can't register archive \"{}\": depot is already initialized.",
                 std::filesystem::relative(aPath, m_gameDir).string());
        return false;
    }

    m_archives.emplace_back(std::move(aPath));
    return true;
}

bool App::ArchiveService::RegisterDirectory(std::filesystem::path aPath)
{
    std::error_code error;

    if (aPath.is_relative())
    {
        aPath = m_gameDir / aPath;
    }

    if (!std::filesystem::exists(aPath, error) || !std::filesystem::is_directory(aPath, error))
    {
        LogError("Can't register archive directory \"{}\": path doesn't exist.",
                 std::filesystem::relative(aPath, m_gameDir).string());
        return false;
    }

    if (m_loaded)
    {
        LogError("Can't register archive directory \"{}\": depot is already initialized.",
                 std::filesystem::relative(aPath, m_gameDir).string());
        return false;
    }

    m_dirs.emplace_back(std::move(aPath));
    return true;
}

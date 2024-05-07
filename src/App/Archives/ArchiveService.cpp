#include "ArchiveService.hpp"
#include "Core/Facades/Runtime.hpp"
#include "Red/ResourceLoader.hpp"

App::ArchiveService::ArchiveService(std::filesystem::path aGameDir)
    : m_gameDir(std::move(aGameDir))
    , m_loaded(false)
{
}

void App::ArchiveService::OnBootstrap()
{
    if (!HookAfter<Raw::ResourceDepot::InitializeArchives>(&ArchiveService::OnInitializeArchives))
        throw std::runtime_error("Failed to hook [ResourceDepot::InitializeArchives].");
}

void App::ArchiveService::OnShutdown()
{
    Unhook<Raw::ResourceDepot::InitializeArchives>();
}

void App::ArchiveService::OnInitializeArchives(Red::ResourceDepot* aDepot)
{
    Red::RegisterPendingTypes();

    m_loaded = true;

    if (m_dirs.empty() && m_archives.empty())
        return;

    LogInfo("Loading archives...");

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
                loadedArchives.push_back(entry.path());
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
    for (auto& group : aDepot->groups)
    {
        if (group.basePath == aBasePath)
        {
            return group;
        }
    }

    aDepot->groups.Emplace(aDepot->groups.Begin() + 1);

    auto& group = aDepot->groups[1];
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
        LogError("Can't register non-existing archive \"{}\".",
                 std::filesystem::relative(aPath, m_gameDir).string());
        return false;
    }

    if (m_loaded)
    {
        LogError("Can't register archive \"{}\" because depot is already initialized.",
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
        LogError("Can't register non-existing archive directory \"{}\".",
                 std::filesystem::relative(aPath, m_gameDir).string());
        return false;
    }

    if (m_loaded)
    {
        LogError("Can't register archive directory \"{}\" because depot is already initialized.",
                 std::filesystem::relative(aPath, m_gameDir).string());
        return false;
    }

    m_dirs.emplace_back(std::move(aPath));
    return true;
}

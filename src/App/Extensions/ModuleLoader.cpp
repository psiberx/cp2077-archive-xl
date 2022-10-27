#include "ModuleLoader.hpp"

App::ModuleLoader::ModuleLoader(std::filesystem::path aConfigDir, std::wstring aConfigExt)
    : m_extraConfigDir(std::move(aConfigDir))
    , m_customConfigExt(std::move(aConfigExt))
    , m_loaded(false)
{
}

bool App::ModuleLoader::ExtraConfigDirExists()
{
    std::error_code error;

    return !m_extraConfigDir.empty() && std::filesystem::exists(m_extraConfigDir, error);
}

void App::ModuleLoader::Configure()
{
    if (m_configurables.empty())
        return;

    try
    {
        LogInfo("Scanning for archive extensions...");

        for (const auto& module : m_configurables)
            module->Reset();

        bool foundAny = false;
        bool successAll = true;

        Core::Vector<std::filesystem::path> configDirs;

        {
            auto depot = Red::ResourceDepot::Get();

            for (const auto& group : depot->groups)
            {
                if (group.scope == Red::ArchiveScope::Mod)
                {
                    configDirs.emplace_back(group.basePath.c_str());
                }
            }
        }

        if (ExtraConfigDirExists())
        {
            configDirs.emplace_back(m_extraConfigDir);
        }

        for (const auto& configDir : configDirs)
        {
            auto configDirIt = std::filesystem::recursive_directory_iterator(
                configDir, std::filesystem::directory_options::follow_directory_symlink);

            for (const auto& entry : configDirIt)
            {
                const auto ext = entry.path().extension();

                if (entry.is_regular_file() && (ext == m_customConfigExt || ext == L".yml" || ext == L".yaml"))
                {
                    successAll &= ReadConfig(entry.path(), configDir);
                    foundAny = true;
                }
            }
        }

        if (foundAny)
        {
            if (successAll)
                LogInfo("Configuration completed.");
            else
                LogWarning("Configuration finished with issues.");
        }
        else
        {
            LogInfo("Nothing found.");
        }
    }
    catch (const std::exception& ex)
    {
        LogError(ex.what());
    }
    catch (...)
    {
        LogError("An unknown error occurred while reading archive extensions.");
    }
}

bool App::ModuleLoader::ReadConfig(const std::filesystem::path& aPath, const std::filesystem::path& aDir)
{
    bool success = true;

    try
    {
        LogInfo("Reading \"{}\"...", std::filesystem::relative(aPath, aDir).string());

        const auto name = aPath.filename().string();
        const auto config = YAML::LoadFile(aPath.string());

        for (const auto& module : m_configurables)
            success &= module->Configure(name, config);
    }
    catch (const std::exception& ex)
    {
        LogError(ex.what());
        success = false;
    }
    catch (...)
    {
        LogError("An unknown error occurred.");
        success = false;
    }

    return success;
}

void App::ModuleLoader::Load()
{
    if (m_loaded)
        return;

    for (const auto& module : m_modules)
    {
        try
        {
            module->Load();
        }
        catch (const std::exception& ex)
        {
            LogError("|{}| {}", module->GetName(), ex.what());
            module->Unload();
        }
        catch (...)
        {
            LogError("|{}| Failed to load. An unknown error has occurred.", module->GetName());
            module->Unload();
        }
    }

    m_loaded = true;
}

void App::ModuleLoader::Unload()
{
    if (!m_loaded)
        return;

    for (const auto& module : m_modules)
    {
        try
        {
            module->Unload();
        }
        catch (const std::exception& ex)
        {
            LogError("|{}| {}", module->GetName(), ex.what());
        }
        catch (...)
        {
            LogError("|{}| Failed to unload. An unknown error has occurred.", module->GetName());
        }
    }

    m_loaded = false;
}

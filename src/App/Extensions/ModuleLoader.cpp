#include "ModuleLoader.hpp"
#include "App/Utils/Str.hpp"

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

void App::ModuleLoader::Configure(const std::filesystem::path& aBundlePath)
{
    if (m_configurables.empty())
        return;

    try
    {
        LogInfo("Configuring archive extensions...");

        for (const auto& module : m_configurables)
            module->Reset();

        if (!aBundlePath.empty())
        {
            ReadConfig(aBundlePath, aBundlePath.parent_path(), true);
        }

        Core::Vector<std::filesystem::path> configDirs;
        Core::Vector<std::filesystem::path> configFiles;

        {
            auto depot = Red::ResourceDepot::Get();

            for (const auto& group : depot->groups)
            {
                if (group.scope == Red::ArchiveScope::Mod)
                {
                    if (group.basePath.Length() > 0)
                    {
                        configDirs.emplace_back(Str::Widen(group.basePath.c_str()));
                    }
                    else
                    {
                        for (const auto& archive : group.archives)
                        {
                            configFiles.emplace_back(Str::Widen(archive.path.c_str()) + m_customConfigExt);
                        }
                    }
                }
            }
        }

        if (!m_extraConfigDir.empty())
        {
            configDirs.emplace_back(m_extraConfigDir);
        }

        bool foundAny = false;
        bool successAll = true;
        std::error_code error;

        for (const auto& configDir : configDirs)
        {
            if (!std::filesystem::exists(configDir, error))
                continue;

            auto configDirIt = std::filesystem::directory_iterator(
                configDir, std::filesystem::directory_options::follow_directory_symlink);

            for (const auto& entry : configDirIt)
            {
                if (entry.is_regular_file() && entry.path().extension() == m_customConfigExt)
                {
                    successAll &= ReadConfig(entry.path(), configDir);
                    foundAny = true;
                }
            }
        }

        for (const auto& configFile : configFiles)
        {
            if (!std::filesystem::exists(configFile, error))
                continue;

            successAll &= ReadConfig(configFile, configFile.parent_path());
            foundAny = true;
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
            LogInfo("No extensions found.");
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

bool App::ModuleLoader::ReadConfig(const std::filesystem::path& aPath, const std::filesystem::path& aDir, bool aSilent)
{
    bool success = true;

    try
    {
        if (!aSilent)
        {
            LogInfo("Reading \"{}\"...", aPath.filename().string());
        }

        std::ifstream file(aPath);

        if (!file)
            throw std::runtime_error("Can't read file.");

        const auto name = aPath.filename().string();
        const auto config = YAML::Load(file);

        for (const auto& module : m_configurables)
            success &= module->Configure(name, config);
    }
    catch (const std::exception& ex)
    {
        LogError("{}: {}", aPath.filename().string(), ex.what());
        success = false;
    }
    catch (...)
    {
        LogError("{}: An unknown error occurred.", aPath.filename().string());
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

void App::ModuleLoader::PostLoad()
{
    if (!m_loaded)
        return;

    for (const auto& module : m_modules)
    {
        try
        {
            module->PostLoad();
        }
        catch (const std::exception& ex)
        {
            LogError("|{}| {}", module->GetName(), ex.what());
            module->Unload();
        }
        catch (...)
        {
            LogError("|{}| Failed to post load. An unknown error has occurred.", module->GetName());
            module->Unload();
        }
    }
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

void App::ModuleLoader::Reload()
{
    if (!m_loaded)
        return;

    for (const auto& module : m_configurables)
    {
        try
        {
            module->Reload();
        }
        catch (const std::exception& ex)
        {
            LogError("|{}| {}", module->GetName(), ex.what());
            module->Unload();
        }
        catch (...)
        {
            LogError("|{}| Failed to reload. An unknown error has occurred.", module->GetName());
            module->Unload();
        }
    }
}

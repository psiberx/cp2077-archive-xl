#include "ModuleLoader.hpp"

App::ModuleLoader::ModuleLoader(std::filesystem::path aConfigDir, std::wstring aConfigExt)
    : m_configDir(std::move(aConfigDir))
    , m_configExt(std::move(aConfigExt))
    , m_loaded(false)
{
}

bool App::ModuleLoader::ConfigDirExists()
{
    std::error_code error;

    return std::filesystem::exists(m_configDir, error);
}

void App::ModuleLoader::Configure()
{
    if (m_configurable.empty() || !ConfigDirExists())
        return;

    try
    {
        LogInfo("Scanning for archive extensions...");

        for (const auto& module : m_configurable)
            module->Reset();

        bool foundAny = false;
        bool successAll = true;

        auto dirIt = std::filesystem::recursive_directory_iterator(
            m_configDir, std::filesystem::directory_options::follow_directory_symlink);

        for (const auto& entry : dirIt)
        {
            const auto ext = entry.path().extension();

            if (entry.is_regular_file() && (ext == m_configExt || ext == L".yml" || ext == L".yaml"))
            {
                successAll &= ReadConfig(entry.path());
                foundAny = true;
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

bool App::ModuleLoader::ReadConfig(const std::filesystem::path& aPath)
{
    bool success = true;

    try
    {
        LogInfo("Reading \"{}\"...", std::filesystem::relative(aPath, m_configDir).string());

        const auto name = aPath.filename().string();
        const auto config = YAML::LoadFile(aPath.string());

        for (const auto& module : m_configurable)
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
            module->Attach();
        }
        catch (const std::exception& ex)
        {
            LogError("|{}| {}", module->GetName(), ex.what());
        }
        catch (...)
        {
            LogError("|{}| Failed to load. An unknown error has occurred.", module->GetName());
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
            module->Detach();
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

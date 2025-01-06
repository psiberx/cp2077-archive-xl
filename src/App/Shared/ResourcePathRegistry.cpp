#include "ResourcePathRegistry.hpp"
#include "App/Project.hpp"
#include "Core/Facades/Container.hpp"
#include "Red/SharedStorage.hpp"

namespace
{
constexpr auto SharedName = Red::CName("ResourcePathRegistryV3" BUILD_SUFFIX);
}

App::ResourcePathRegistry::ResourcePathRegistry(const std::filesystem::path& aPreloadPath)
{
    s_instance = Red::AcquireSharedInstance<SharedName, SharedInstance>();
    s_preloadPath = aPreloadPath;
}

void App::ResourcePathRegistry::OnBootstrap()
{
    std::unique_lock lock(s_instance->m_lock);

    if (!s_instance->m_initialized)
    {
        s_instance->m_initialized = true;
        s_instance->m_map.reserve(400000);

        HookAfter<Raw::ResourcePath::Create>(&OnCreatePath);
    }

    if (!s_instance->m_preloaded && !s_preloadPath.empty() && std::filesystem::exists(s_preloadPath))
    {
        s_instance->m_preloaded = true;

        std::thread([lock = std::move(lock)]() {
            LogInfo("[ResourcePathRegistry] Loading metadata...");

            std::ifstream f(s_preloadPath);
            std::string s;
            while (std::getline(f, s))
            {
                s_instance->m_map[Red::ResourcePath::HashSanitized(s.data())] = std::move(s);
            }

            LogInfo("[ResourcePathRegistry] Loaded {} predefined hashes.", s_instance->m_map.size());
        }).detach();
    }
}

void App::ResourcePathRegistry::OnCreatePath(Red::ResourcePath* aPath, Red::StringView* aPathStr)
{
    if (aPathStr)
    {
        std::scoped_lock _(s_instance->m_lock);
        s_instance->m_map[*aPath] = {aPathStr->data, aPathStr->size};
    }
}

std::string App::ResourcePathRegistry::ResolvePath(Red::ResourcePath aPath)
{
    if (!aPath)
        return {};

    std::shared_lock _(s_instance->m_lock);
    const auto& it = s_instance->m_map.find(aPath);

    if (it == s_instance->m_map.end())
        return {};

    return it.value();
}

std::string App::ResourcePathRegistry::ResolvePathOrHash(Red::ResourcePath aPath)
{
    auto str = ResolvePath(aPath);

    if (str.empty())
    {
        str = std::to_string(aPath.hash);
    }

    return str;
}

void App::ResourcePathRegistry::RegisterPath(Red::ResourcePath aPath, const std::string& aPathStr)
{
    if (!aPath)
        return;

    {
        std::shared_lock _(s_instance->m_lock);
        if (s_instance->m_map.contains(aPath))
            return;
    }

    {
        std::scoped_lock _(s_instance->m_lock);
        s_instance->m_map[aPath] = aPathStr;
    }
}

App::ResourcePathRegistry* App::ResourcePathRegistry::Get()
{
    static const auto s_reference = Core::Resolve<ResourcePathRegistry>();
    return s_reference.get();
}

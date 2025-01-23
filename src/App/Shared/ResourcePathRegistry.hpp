#pragma once

#include "Core/Foundation/Feature.hpp"
#include "Core/Hooking/HookingAgent.hpp"
#include "Core/Logging/LoggingAgent.hpp"
#include "Red/ResourcePath.hpp"

namespace App
{
class ResourcePathRegistry
    : public Core::Feature
    , public Core::LoggingAgent
    , public Core::HookingAgent
{
public:
    ResourcePathRegistry(const std::filesystem::path& aPreloadPath = {});

    [[nodiscard]] std::string ResolvePath(Red::ResourcePath aPath);
    [[nodiscard]] std::string ResolvePathOrHash(Red::ResourcePath aPath);

    Red::ResourcePath RegisterPath(const std::string& aPathStr);
    void RegisterPath(Red::ResourcePath aPath, const std::string& aPathStr);

protected:
    struct SharedInstance
    {
        Red::SharedSpinLock m_lock;
        Core::Map<Red::ResourcePath, std::string> m_map;
        bool m_preloaded{false};
        bool m_initialized{false};
    };

    void OnBootstrap() override;
    static void OnCreatePath(Red::ResourcePath* aPath, Red::StringView* aPathStr);

    inline static SharedInstance* s_instance;
    inline static std::filesystem::path s_preloadPath;
};
}

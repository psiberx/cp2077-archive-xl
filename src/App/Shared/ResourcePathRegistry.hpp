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

    [[nodiscard]] std::string_view ResolvePath(Red::ResourcePath aPath);

    static ResourcePathRegistry* Get();

protected:
    struct SharedInstance
    {
        std::shared_mutex m_mutex;
        Core::Map<Red::ResourcePath, std::string> m_map;
        bool m_preloaded{false};
        bool m_hooked{false};
    };

    void OnBootstrap() override;
    static void OnCreatePath(Red::ResourcePath* aPath, Red::StringView* aPathStr);

    inline static SharedInstance* s_instance;
    inline static std::filesystem::path s_preloadPath;
};
}

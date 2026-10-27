#pragma once

#include "Core/Facades/Runtime.hpp"

namespace App
{
class Config
{
public:
    static inline bool DebugLogging()
    {
#ifdef NDEBUG
        Load();

        return s_config["logging"]["debug"].as<bool>(false);
#else
        return true;
#endif
    }

private:
    static inline void Load()
    {
        if (!s_config.IsDefined())
        {
            const auto path = Core::Runtime::GetModuleDir() / L"ArchiveXL.yaml";

            std::error_code error;
            if (std::filesystem::exists(path) && std::filesystem::is_regular_file(path, error))
            {
                s_config = YAML::LoadFile(path.string());
            }
            else
            {
                s_config = {};
            }
        }
    }

    static inline YAML::Node s_config{YAML::NodeType::Undefined};
};
}

#pragma once

#include "Core/Facades/Runtime.hpp"

namespace App::Env
{
inline std::filesystem::path GameDir()
{
    return Core::Runtime::GetRootDir();
}

inline std::filesystem::path ArchiveDir()
{
    return Core::Runtime::GetModuleDir() / L"Archive";
}

inline std::filesystem::path ScriptsDir()
{
    return Core::Runtime::GetModuleDir() / L"Scripts";
}

inline std::filesystem::path LegacyBundleDir()
{
    return Core::Runtime::GetModuleDir() / L"Bundle";
}

inline std::filesystem::path LegacyScriptsDir()
{
    return Core::Runtime::GetRootDir() / L"r6" / L"scripts" / L"ArchiveXL";
}
}

#pragma once

#include "Core/Facades/Runtime.hpp"

namespace App::Configuration
{
inline std::filesystem::path GetModArchiveDir()
{
    return Core::Runtime::GetRootDir() / L"archive" / L"pc" / L"mod";
}
}

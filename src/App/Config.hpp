#pragma once

#include "Core/Facades/Runtime.hpp"

namespace App::Config
{
inline std::filesystem::path GetModArchiveDir()
{
    return Core::Runtime::GetRootDir() / L"archive" / L"pc" / L"mod";
}
}

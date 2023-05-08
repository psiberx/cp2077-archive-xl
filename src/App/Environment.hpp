#pragma once

#include "Core/Facades/Runtime.hpp"

namespace App::Env
{
inline std::filesystem::path GameDir()
{
    return Core::Runtime::GetRootDir();
}
}

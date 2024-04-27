#pragma once

namespace App::Migration
{
inline void CleanUp(const std::filesystem::path& aPath)
{
    std::error_code error;
    if (std::filesystem::exists(aPath, error))
    {
        std::filesystem::remove_all(aPath, error);
    }
}
}

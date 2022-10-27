#pragma once

#include "Core/Win.hpp"

namespace Core
{
class ModuleImage
{
public:
    explicit ModuleImage(HMODULE aHandle);
    ~ModuleImage() = default;

    [[nodiscard]] std::filesystem::path GetPath() const;
    [[nodiscard]] std::filesystem::path GetDir() const;
    [[nodiscard]] std::string GetName() const;
    [[nodiscard]] bool IsASI() const;

private:
    std::filesystem::path m_path;
};
}

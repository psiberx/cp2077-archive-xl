#pragma once

namespace Core
{
class HostImage
{
public:
    explicit HostImage(int32_t aExePathDepth = 0);
    ~HostImage() = default;

    [[nodiscard]] uintptr_t GetBase() const;
    [[nodiscard]] std::filesystem::path GetPath() const;
    [[nodiscard]] std::string GetName() const;
    [[nodiscard]] std::filesystem::path GetRootDir() const;

private:
    uintptr_t m_base;
    std::filesystem::path m_exe;
    std::filesystem::path m_root;
};
}

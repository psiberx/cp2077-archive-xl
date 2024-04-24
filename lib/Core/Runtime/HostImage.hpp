#pragma once

namespace Core
{
struct FileVer
{
    uint16_t major;
    uint16_t minor;
    uint16_t build;
    uint16_t revision;
};

struct SemvVer
{
    uint16_t major;
    uint16_t minor;
    uint32_t patch;
};

class HostImage
{
public:
    explicit HostImage(int32_t aExePathDepth = 0);
    ~HostImage() = default;

    [[nodiscard]] uintptr_t GetBase() const;
    [[nodiscard]] std::filesystem::path GetPath() const;
    [[nodiscard]] std::string GetName() const;
    [[nodiscard]] std::filesystem::path GetRootDir() const;

    [[nodiscard]] const FileVer& GetFileVer() const;
    [[nodiscard]] const SemvVer& GetProductVer() const;

private:
    bool TryResolveVersion(const std::wstring& aFilePath);

    uintptr_t m_base;
    std::filesystem::path m_exe;
    std::filesystem::path m_root;
    FileVer m_fileVer{};
    SemvVer m_productVer{};
};
}

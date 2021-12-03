#pragma once

class Environment
{
public:
    Environment();
    ~Environment() = default;

    [[nodiscard]] std::filesystem::path GetGameExe() const;
    [[nodiscard]] std::filesystem::path GetRootDir() const;
    [[nodiscard]] std::filesystem::path GetArchiveModDir() const;

private:
    std::filesystem::path m_exe;
    std::filesystem::path m_root;
};

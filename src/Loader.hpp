#pragma once

#include "Extension.hpp"
#include "Environment.hpp"

namespace AXL {
class Loader {
public:
    static constexpr auto ConfigExtension = L".xl";

    ~Loader();

    static void Construct();
    static void Destruct();
    static Loader *Get();

private:
    Loader();

    void Configure();
    void Attach();
    void Detach();

    Environment m_env;
    std::vector<std::unique_ptr<Extension>> m_extensions;
    //spdlog:: m_logger;
};
}

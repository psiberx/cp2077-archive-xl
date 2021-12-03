#pragma once

#include "Hooking/HookAgent.hpp"

namespace AXL
{
class Extension: public HookAgent
{
public:
    Extension() = default;
    virtual ~Extension() = default;

    virtual void Configure(const std::string& aName, const YAML::Node& aNode) {};
    virtual void Attach() {};
    virtual void Detach() {};

private:
    //spdlog:: m_logger;
};
}

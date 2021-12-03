#pragma once

namespace AXL
{
struct Definition
{
    Definition(const std::string& aName)
        : name(aName) {}

    std::string name;

    virtual bool IsDefined() = 0;
};
}

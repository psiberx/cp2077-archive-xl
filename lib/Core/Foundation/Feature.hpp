#pragma once

namespace Core
{
class Feature
{
public:
    Feature() = default;
    virtual ~Feature() = default;

    Feature(Feature&& aOther) = delete;
    Feature(const Feature& aOther) = delete;

    virtual void Bootstrap() {};
    virtual void Shutdown() {};
};
}

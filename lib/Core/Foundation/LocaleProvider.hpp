#pragma once

#include "Core/Foundation/Feature.hpp"

namespace Core
{
class LocaleProvider : public Feature
{
public:
    LocaleProvider(const char* aLocale = "en_US.UTF-8")
    {
        std::setlocale(LC_ALL, aLocale);
    }
};
}

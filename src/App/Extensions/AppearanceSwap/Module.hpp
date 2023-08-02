#pragma once

#include "App/Extensions/ModuleBase.hpp"
#include "Red/AppearanceChanger.hpp"

namespace App
{
class AppearanceSwapModule : public Module
{
public:
    bool Load() override;
    bool Unload() override;
    std::string_view GetName() override;

private:
    static bool OnLoadTemplate(Red::ItemFactoryAppearanceChangeRequest* aRequest);
};
}

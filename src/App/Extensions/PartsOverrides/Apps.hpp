#pragma once

namespace App
{
class DynamicAppearanceResolver
{
public:
    [[nodiscard]] Red::CName GetAppearance(Red::ent::Entity* aEntity, Red::CName aAppearance);

    [[nodiscard]] static Core::SharedPtr<DynamicAppearanceResolver>& Get();

private:
    const char* GetTagValue(Red::CName aTag, Red::ent::Entity* aEntity);
    Red::CName GetSkinColor(Red::ent::Entity* aEntity);
    Red::CName GetHairColor(Red::ent::Entity* aEntity);
    bool IsMale(Red::ent::Entity* aEntity);
};
}

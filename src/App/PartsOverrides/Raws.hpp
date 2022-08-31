#pragma once

#include "Garment.hpp"
#include "Core/Raw.hpp"
#include "Reverse/Addresses.hpp"

#include <RED4ext/Memory/SharedPtr.hpp>
#include <RED4ext/Scripting/Natives/Generated/ent/Entity.hpp>

namespace App::Raw
{
namespace AppearanceChanger
{
using ComputePlayerGarment = Core::RawFunc<
    /* address = */ Reverse::Addresses::AppearanceChanger_ComputePlayerGarment,
    /* signature = */ void (*)(RED4ext::Handle<RED4ext::ent::Entity>&, uintptr_t,
                               RED4ext::SharedPtr<GarmentComputeData>&, uintptr_t, uintptr_t, uintptr_t, bool)>;
}

namespace Entity
{
using GetComponents = Core::RawFunc<
    /* address = */ Reverse::Addresses::Entity_GetComponents,
    /* signature = */ RED4ext::DynArray<RED4ext::Handle<RED4ext::ent::IComponent>>& (*)(RED4ext::ent::Entity*)>;

using ReassembleAppearance = Core::RawFunc<
    /* address = */ Reverse::Addresses::Entity_ReassembleAppearance,
    /* signature = */ void (*)(RED4ext::ent::Entity*, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t)>;
}

namespace GarmentAssembler
{
using AddItem = Core::RawFunc<
    /* address = */ Reverse::Addresses::GarmentAssembler_AddItem,
    /* signature = */ bool (*)(uintptr_t, RED4ext::WeakHandle<RED4ext::ent::Entity>&, GarmentItemAddRequest&)>;

using OverrideItem = Core::RawFunc<
    /* address = */ Reverse::Addresses::GarmentAssembler_OverrideItem,
    /* signature = */ bool (*)(uintptr_t, RED4ext::WeakHandle<RED4ext::ent::Entity>&, GarmentItemOverrideRequest&)>;

using RemoveItem = Core::RawFunc<
    /* address = */ Reverse::Addresses::GarmentAssembler_RemoveItem,
    /* signature = */ bool (*)(uintptr_t, RED4ext::WeakHandle<RED4ext::ent::Entity>&, GarmentItemRemoveRequest&)>;

using OnGameDetach = Core::RawFunc<
    /* address = */ Reverse::Addresses::GarmentAssembler_OnGameDetach,
    /* signature = */ void (*)(uintptr_t)>;
}
}

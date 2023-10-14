#pragma once

#include "Core/Raw.hpp"
#include "Red/Addresses.hpp"

namespace Raw::CMesh
{
constexpr auto GetAppearance = Core::RawFunc<
    /* addr = */ Red::Addresses::CMesh_GetAppearance,
    /* type = */ Red::Handle<Red::mesh::MeshAppearance>& (*)(Red::CMesh* aMesh, Red::CName aAppearance)>();
}

namespace Raw::MeshAppearance
{
constexpr auto LoadMaterialSetupAsync = Core::RawFunc<
    /* addr = */ Red::Addresses::MeshAppearance_LoadMaterialSetupAsync,
    /* type = */ void (*)(Red::mesh::MeshAppearance& aAppearance, Red::Handle<Red::mesh::MeshAppearance>& aOut,
                          uint8_t a3)>();
}

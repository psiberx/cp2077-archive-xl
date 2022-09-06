#pragma once

#include "Addresses.hpp"
#include "Core/Raw.hpp"

#include <RED4ext/Addresses.hpp>

namespace TweakDB::Raw
{
using Core::RawFunc;

using LoadTweakDB = RawFunc<Addresses::TweakDB_Load,
    void (*)(RED4ext::TweakDB*, RED4ext::CString&)>;

using CreateRecord = RawFunc<RED4ext::Addresses::TweakDB_CreateRecord,
    void (*)(RED4ext::TweakDB*, uint32_t, RED4ext::TweakDBID)>;

using CreateTweakDBID = RawFunc<Addresses::TweakDBID_Derive,
    void (*)(RED4ext::TweakDBID*, const RED4ext::TweakDBID*, const char*)>;
}

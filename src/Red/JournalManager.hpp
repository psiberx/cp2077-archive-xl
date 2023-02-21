#pragma once

#include "Core/Raw.hpp"
#include "Red/Addresses.hpp"

namespace Raw::JournalManager
{
constexpr auto LoadJournal = Core::RawFunc<
    /* addr = */ Red::Addresses::JournalManager_LoadJournal,
    /* type = */ void (*)(Red::IScriptable* aManager, Red::JobHandle& aJobHandle)>();

constexpr auto TrackQuest = Core::RawFunc<
    /* addr = */ Red::Addresses::JournalManager_TrackQuest,
    /* type = */ void (*)(Red::IScriptable* aManager, Red::Handle<Red::game::JournalEntry>& aEntry)>();

constexpr auto TrackPointOfInterest = Core::RawFunc<
    /* addr = */ Red::Addresses::JournalManager_TrackPointOfInterest,
    /* type = */ void (*)(Red::IScriptable* aManager, Red::Handle<Red::game::JournalEntry>& aEntry)>();
}

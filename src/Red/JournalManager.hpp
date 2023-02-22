#pragma once

#include "Core/Raw.hpp"
#include "Red/Addresses.hpp"

namespace Red
{
using JournalEntryHash = uint32_t;
}

namespace Raw::JournalManager
{
constexpr auto LoadJournal = Core::RawFunc<
    /* addr = */ Red::Addresses::JournalManager_LoadJournal,
    /* type = */ void (*)(Red::IScriptable* aManager, Red::JobHandle& aJobHandle)>();

constexpr auto GetEntryHash = Core::RawFunc<
    /* addr = */ Red::Addresses::JournalManager_GetEntryHash,
    /* type = */ Red::JournalEntryHash (*)(Red::IScriptable* aManager, Red::Handle<Red::game::JournalEntry>& aEntry)>();

constexpr auto GetEntryByHash = Core::RawFunc<
    /* addr = */ Red::Addresses::JournalManager_GetEntryByHash,
    /* type = */ void (*)(Red::IScriptable* aManager, Red::Handle<Red::game::JournalEntry>& aEntry,
                          Red::JournalEntryHash aHash)>();

constexpr auto GetTrackedQuest = Core::RawFunc<
    /* addr = */ Red::Addresses::JournalManager_GetTrackedQuest,
    /* type = */ void (*)(Red::IScriptable* aManager, Red::Handle<Red::game::JournalEntry>& aEntry)>();

constexpr auto GetTrackedPointOfInterest = Core::RawFunc<
    /* addr = */ Red::Addresses::JournalManager_GetTrackedPointOfInterest,
    /* type = */ void (*)(Red::IScriptable* aManager, Red::Handle<Red::game::JournalEntry>& aEntry)>();

constexpr auto TrackQuest = Core::RawFunc<
    /* addr = */ Red::Addresses::JournalManager_TrackQuest,
    /* type = */ void (*)(Red::IScriptable* aManager, Red::Handle<Red::game::JournalEntry>& aEntry)>();

constexpr auto TrackPointOfInterest = Core::RawFunc<
    /* addr = */ Red::Addresses::JournalManager_TrackPointOfInterest,
    /* type = */ void (*)(Red::IScriptable* aManager, Red::Handle<Red::game::JournalEntry>& aEntry)>();
}

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

constexpr auto GetTrackedQuest = Core::RawVFunc<
    /* addr = */ 0x1F8,
    /* type = */ void (Red::gameIJournalManager::*)(Red::Handle<Red::game::JournalEntry>& aEntry)>();

constexpr auto GetTrackedPointOfInterest = Core::RawVFunc<
    /* addr = */ 0x208,
    /* type = */ void (Red::gameIJournalManager::*)(Red::Handle<Red::game::JournalEntry>& aEntry)>();

constexpr auto GetEntryByHash = Core::RawVFunc<
    /* addr = */ 0x220,
    /* type = */ void* (Red::gameIJournalManager::*)(Red::Handle<Red::gameJournalEntry>& aOut,
                                                     Red::JournalEntryHash aEntryHash)>();

constexpr auto GetEntryHash = Core::RawVFunc<
    /* addr = */ 0x230,
    /* type = */ Red::JournalEntryHash (Red::gameIJournalManager::*)(Red::Handle<Red::game::JournalEntry>& aEntry)>();

// constexpr auto ChangeEntryState = Core::RawVFunc<
//     /* addr = */ 0x268,
//     /* type = */ bool (Red::gameIJournalManager::*)(Red::Handle<Red::gameJournalEntry>& aEntry,
//                                                     Red::gameJournalEntryState aEntryState,
//                                                     Red::gameJournalNotifyOption aNotifyOption,
//                                                     uint32_t a4)>();

constexpr auto TrackQuest = Core::RawFunc<
    /* addr = */ Red::Addresses::JournalManager_TrackQuest,
    /* type = */ void (Red::gameIJournalManager::*)(Red::Handle<Red::game::JournalEntry>& aEntry)>();

constexpr auto TrackQuestByPath = Core::RawVFunc<
    /* addr = */ 0x298,
    /* type = */ void (Red::gameIJournalManager::*)(void* aPath)>();

constexpr auto TrackPointOfInterest = Core::RawVFunc<
    /* addr = */ 0x2A0,
    /* type = */ void (Red::gameIJournalManager::*)(Red::Handle<Red::game::JournalEntry>& aEntry)>();

// constexpr auto GetEntryState = Core::RawVFunc<
//     /* addr = */ 0x2B0,
//     /* type = */ Red::gameJournalEntryState (Red::gameIJournalManager::*)(Red::Handle<Red::gameJournalEntry>& aEntry)>();
//
// constexpr auto GetEntryTimestamp = Core::RawVFunc<
//     /* addr = */ 0x2C8,
//     /* type = */ uint32_t (Red::gameIJournalManager::*)(Red::Handle<Red::gameJournalEntry>& aEntry)>();
}

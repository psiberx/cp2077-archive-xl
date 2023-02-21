#pragma once

#include "Core/Raw.hpp"
#include "Red/Addresses.hpp"

namespace Raw::JournalTree
{
constexpr auto ProcessJournalIndex = Core::RawFunc<
    /* addr = */ Red::Addresses::JournalTree_ProcessJournalIndex,
    /* type = */ void (*)(uintptr_t a1, Red::JobGroup& aJobGroup)>();
}

namespace Raw::JournalRootFolderEntry
{
constexpr auto Initialize = Core::RawFunc<
    /* addr = */ Red::Addresses::JournalRootFolderEntry_Initialize,
    /* type = */ void (*)(Red::game::JournalRootFolderEntry* aRoot,
                          uintptr_t a2, uintptr_t a3, Red::JobQueue& aJobQueue)>();
}

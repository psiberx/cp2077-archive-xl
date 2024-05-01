#pragma once

namespace Red
{
template<typename T>
struct Range
{
    constexpr operator bool() const noexcept
    {
        return beginPtr != endPtr;
    }

    [[nodiscard]] inline T* begin() const
    {
        return beginPtr;
    }

    [[nodiscard]] inline T* end() const
    {
        return endPtr;
    }

    [[nodiscard]] auto GetSize() const
    {
        return endPtr - beginPtr;
    }

    [[nodiscard]] bool IsEmpty() const
    {
        return !beginPtr;
    }

    T* beginPtr{}; // 00
    T* endPtr{};   // 08
};
RED4EXT_ASSERT_SIZE(Range<int32_t>, 0x10);
RED4EXT_ASSERT_OFFSET(Range<int32_t>, beginPtr, 0x0);
RED4EXT_ASSERT_OFFSET(Range<int32_t>, endPtr, 0x8);
}

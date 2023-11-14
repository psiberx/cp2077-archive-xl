#pragma once

namespace Red
{
template<typename T>
struct Range
{
    constexpr operator bool() const noexcept
    {
        return ptr != end;
    }

    constexpr operator T*() const noexcept
    {
        return ptr;
    }

    constexpr operator T&() const noexcept
    {
        return *ptr;
    }

    [[nodiscard]] auto GetSize() const
    {
        return end - ptr;
    }

    T* ptr; // 00
    T* end; // 08
};
RED4EXT_ASSERT_SIZE(Range<int32_t>, 0x10);
RED4EXT_ASSERT_OFFSET(Range<int32_t>, ptr, 0x0);
RED4EXT_ASSERT_OFFSET(Range<int32_t>, end, 0x8);
}

#pragma once

namespace Core
{
enum class AddressSegment : uint32_t
{
    Text = 0,
    Data,
    Rdata
};
}

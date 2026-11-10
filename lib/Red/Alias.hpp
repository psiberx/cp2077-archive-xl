#pragma once

namespace Red
{
using namespace RED4ext;

template<typename T>
using ResourceTokenPtr = SharedPtr<ResourceToken<T>>; // TODO: Move to SDK
}

namespace Red::Detail
{
using namespace RED4ext::Detail;
}

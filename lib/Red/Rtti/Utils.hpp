#pragma once

namespace Red
{
CName ToArrayType(CName aType)
{
    return std::string("array:").append(aType.ToString()).c_str();
}
}

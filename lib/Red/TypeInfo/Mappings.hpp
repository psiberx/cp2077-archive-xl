#pragma once

#include "Resolving.hpp"

namespace Red
{
RTTI_MAP_TYPE_NAME(int8_t, "Int8");
RTTI_MAP_TYPE_NAME(uint8_t, "Uint8");
RTTI_MAP_TYPE_NAME(int16_t, "Int16");
RTTI_MAP_TYPE_NAME(uint16_t, "Uint16");
RTTI_MAP_TYPE_NAME(int32_t, "Int32");
RTTI_MAP_TYPE_NAME(uint32_t, "Uint32");
RTTI_MAP_TYPE_NAME(int64_t, "Int64");
RTTI_MAP_TYPE_NAME(uint64_t, "Uint64");
RTTI_MAP_TYPE_NAME(float, "Float");
RTTI_MAP_TYPE_NAME(double, "Double");
RTTI_MAP_TYPE_NAME(bool, "Bool");
RTTI_MAP_TYPE_NAME(CString, "String");
RTTI_MAP_TYPE_NAME(CName, "CName");
RTTI_MAP_TYPE_NAME(TweakDBID, "TweakDBID");
RTTI_MAP_TYPE_NAME(ItemID, "gameItemID");
RTTI_MAP_TYPE_NAME(NodeRef, "NodeRef");
RTTI_MAP_TYPE_NAME(GlobalNodeRef, "worldGlobalNodeRef");
RTTI_MAP_TYPE_NAME(Variant, "Variant");

RTTI_MAP_TYPE_PREFIX(DynArray, "array:");
RTTI_MAP_TYPE_PREFIX(Handle, "handle:");
RTTI_MAP_TYPE_PREFIX(WeakHandle, "whandle:");
RTTI_MAP_TYPE_PREFIX(ResourceReference, "rRef:");
RTTI_MAP_TYPE_PREFIX(ResourceAsyncReference, "raRef:");
RTTI_MAP_TYPE_PREFIX(CurveData, "curveData:");

RTTI_MAP_TYPE_NAME(char, "Uint8");
RTTI_MAP_TYPE_NAME(ResourcePath, "redResourceReferenceScriptToken");
}

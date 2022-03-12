#pragma once

#include "stdafx.hpp"

namespace App::Language
{
enum Code : uint64_t
{
    Arabic = (uint64_t) RED4ext::CName("ar-ar"),
    Czech = (uint64_t) RED4ext::CName("cz-cz"),
    German = (uint64_t) RED4ext::CName("de-de"),
    English = (uint64_t) RED4ext::CName("en-us"),
    Spanish = (uint64_t) RED4ext::CName("es-es"),
    LatinAmericanSpanish = (uint64_t) RED4ext::CName("es-mx"),
    French = (uint64_t) RED4ext::CName("fr-fr"),
    Hungarian = (uint64_t) RED4ext::CName("hu-hu"),
    Italian = (uint64_t) RED4ext::CName("it-it"),
    Japanese = (uint64_t) RED4ext::CName("jp-jp"),
    Korean = (uint64_t) RED4ext::CName("kr-kr"),
    Polish = (uint64_t) RED4ext::CName("pl-pl"),
    BrazilianPortuguese = (uint64_t) RED4ext::CName("pt-br"),
    Russian = (uint64_t) RED4ext::CName("ru-ru"),
    Thai = (uint64_t) RED4ext::CName("th-th"),
    Turkish = (uint64_t) RED4ext::CName("tr-tr"),
    SimplifiedChinese = (uint64_t) RED4ext::CName("zh-cn"),
    TraditionalChinese = (uint64_t) RED4ext::CName("zh-tw"),
};

bool IsKnown(RED4ext::CName aLanguage);
RED4ext::CName ResolveFromResource(Engine::ResourcePath aPath);
}

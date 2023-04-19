#pragma once

namespace App::Language
{
enum Code : uint64_t
{
    Arabic = (uint64_t) Red::CName("ar-ar"),
    Czech = (uint64_t) Red::CName("cz-cz"),
    German = (uint64_t) Red::CName("de-de"),
    English = (uint64_t) Red::CName("en-us"),
    Spanish = (uint64_t) Red::CName("es-es"),
    LatinAmericanSpanish = (uint64_t) Red::CName("es-mx"),
    French = (uint64_t) Red::CName("fr-fr"),
    Hungarian = (uint64_t) Red::CName("hu-hu"),
    Italian = (uint64_t) Red::CName("it-it"),
    Japanese = (uint64_t) Red::CName("jp-jp"),
    Korean = (uint64_t) Red::CName("kr-kr"),
    Polish = (uint64_t) Red::CName("pl-pl"),
    BrazilianPortuguese = (uint64_t) Red::CName("pt-br"),
    Russian = (uint64_t) Red::CName("ru-ru"),
    Thai = (uint64_t) Red::CName("th-th"),
    Turkish = (uint64_t) Red::CName("tr-tr"),
    SimplifiedChinese = (uint64_t) Red::CName("zh-cn"),
    TraditionalChinese = (uint64_t) Red::CName("zh-tw"),
};

bool IsKnown(Red::CName aLanguage);
Red::CName ResolveFromTextResource(Red::ResourcePath aPath);
Red::CName ResolveFromSubtitleResource(Red::ResourcePath aPath);
Red::CName ResolveFromLipsyncResource(Red::ResourcePath aPath);
}

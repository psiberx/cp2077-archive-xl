#include "Language.hpp"

bool App::Language::IsKnown(Red::CName aLanguage)
{
    switch (aLanguage)
    {
    case Code::Arabic:
    case Code::Czech:
    case Code::German:
    case Code::English:
    case Code::Spanish:
    case Code::LatinAmericanSpanish:
    case Code::French:
    case Code::Hungarian:
    case Code::Italian:
    case Code::Japanese:
    case Code::Korean:
    case Code::Polish:
    case Code::BrazilianPortuguese:
    case Code::Russian:
    case Code::Thai:
    case Code::Turkish:
    case Code::SimplifiedChinese:
    case Code::TraditionalChinese:
        return true;
    default:
        return false;
    }
}

Red::CName App::Language::ResolveFromTextResource(Red::ResourcePath aPath)
{
    static Core::Map<Red::ResourcePath, Red::CName> s_codes {
        { R"(base\localization\ar-ar\onscreens\onscreens.json)", "ar-ar" },
        { R"(base\localization\cz-cz\onscreens\onscreens.json)", "cz-cz" },
        { R"(base\localization\db-db\onscreens\onscreens.json)", "db-db" },
        { R"(base\localization\de-de\onscreens\onscreens.json)", "de-de" },
        { R"(base\localization\en-us\onscreens\onscreens.json)", "en-us" },
        { R"(base\localization\es-es\onscreens\onscreens.json)", "es-es" },
        { R"(base\localization\es-mx\onscreens\onscreens.json)", "es-mx" },
        { R"(base\localization\fr-fr\onscreens\onscreens.json)", "fr-fr" },
        { R"(base\localization\hu-hu\onscreens\onscreens.json)", "hu-hu" },
        { R"(base\localization\it-it\onscreens\onscreens.json)", "it-it" },
        { R"(base\localization\jp-jp\onscreens\onscreens.json)", "jp-jp" },
        { R"(base\localization\kr-kr\onscreens\onscreens.json)", "kr-kr" },
        { R"(base\localization\pl-pl\onscreens\onscreens.json)", "pl-pl" },
        { R"(base\localization\pt-br\onscreens\onscreens.json)", "pt-br" },
        { R"(base\localization\ru-ru\onscreens\onscreens.json)", "ru-ru" },
        { R"(base\localization\th-th\onscreens\onscreens.json)", "th-th" },
        { R"(base\localization\tr-tr\onscreens\onscreens.json)", "tr-tr" },
        { R"(base\localization\zh-cn\onscreens\onscreens.json)", "zh-cn" },
        { R"(base\localization\zh-tw\onscreens\onscreens.json)", "zh-tw" },
    };

    const auto& it = s_codes.find(aPath);

    if (it == s_codes.end())
        return {};

    return it.value();
}

Red::CName App::Language::ResolveFromSubtitleResource(Red::ResourcePath aPath)
{
    static Core::Map<Red::ResourcePath, Red::CName> s_codes {
        { R"(base\localization\ar-ar\subtitles\subtitles.json)", "ar-ar" },
        { R"(base\localization\cz-cz\subtitles\subtitles.json)", "cz-cz" },
        { R"(base\localization\db-db\subtitles\subtitles.json)", "db-db" },
        { R"(base\localization\de-de\subtitles\subtitles.json)", "de-de" },
        { R"(base\localization\en-us\subtitles\subtitles.json)", "en-us" },
        { R"(base\localization\es-es\subtitles\subtitles.json)", "es-es" },
        { R"(base\localization\es-mx\subtitles\subtitles.json)", "es-mx" },
        { R"(base\localization\fr-fr\subtitles\subtitles.json)", "fr-fr" },
        { R"(base\localization\hu-hu\subtitles\subtitles.json)", "hu-hu" },
        { R"(base\localization\it-it\subtitles\subtitles.json)", "it-it" },
        { R"(base\localization\jp-jp\subtitles\subtitles.json)", "jp-jp" },
        { R"(base\localization\kr-kr\subtitles\subtitles.json)", "kr-kr" },
        { R"(base\localization\pl-pl\subtitles\subtitles.json)", "pl-pl" },
        { R"(base\localization\pt-br\subtitles\subtitles.json)", "pt-br" },
        { R"(base\localization\ru-ru\subtitles\subtitles.json)", "ru-ru" },
        { R"(base\localization\th-th\subtitles\subtitles.json)", "th-th" },
        { R"(base\localization\tr-tr\subtitles\subtitles.json)", "tr-tr" },
        { R"(base\localization\zh-cn\subtitles\subtitles.json)", "zh-cn" },
        { R"(base\localization\zh-tw\subtitles\subtitles.json)", "zh-tw" },
    };

    const auto& it = s_codes.find(aPath);

    if (it == s_codes.end())
        return {};

    return it.value();
}

Red::CName App::Language::ResolveFromLipsyncResource(Red::ResourcePath aPath)
{
    static Core::Map<Red::ResourcePath, Red::CName> s_codes {
        { R"(base\localization\ar-ar.lipmap)", "ar-ar" },
        { R"(base\localization\cz-cz.lipmap)", "cz-cz" },
        { R"(base\localization\db-db.lipmap)", "db-db" },
        { R"(base\localization\de-de.lipmap)", "de-de" },
        { R"(base\localization\en-us.lipmap)", "en-us" },
        { R"(base\localization\es-es.lipmap)", "es-es" },
        { R"(base\localization\es-mx.lipmap)", "es-mx" },
        { R"(base\localization\fr-fr.lipmap)", "fr-fr" },
        { R"(base\localization\hu-hu.lipmap)", "hu-hu" },
        { R"(base\localization\it-it.lipmap)", "it-it" },
        { R"(base\localization\jp-jp.lipmap)", "jp-jp" },
        { R"(base\localization\kr-kr.lipmap)", "kr-kr" },
        { R"(base\localization\pl-pl.lipmap)", "pl-pl" },
        { R"(base\localization\pt-br.lipmap)", "pt-br" },
        { R"(base\localization\ru-ru.lipmap)", "ru-ru" },
        { R"(base\localization\th-th.lipmap)", "th-th" },
        { R"(base\localization\tr-tr.lipmap)", "tr-tr" },
        { R"(base\localization\zh-cn.lipmap)", "zh-cn" },
        { R"(base\localization\zh-tw.lipmap)", "zh-tw" },
    };

    const auto& it = s_codes.find(aPath);

    if (it == s_codes.end())
        return {};

    return it.value();
}

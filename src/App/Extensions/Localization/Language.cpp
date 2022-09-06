#include "Language.hpp"

bool App::Language::IsKnown(RED4ext::CName aLanguage)
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

RED4ext::CName App::Language::ResolveFromResource(RED4ext::ResourcePath aPath)
{
    static Core::Map<RED4ext::ResourcePath, RED4ext::CName> s_codes {
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

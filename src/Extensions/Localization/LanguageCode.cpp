#include "stdafx.hpp"
#include "LanguageCode.hpp"

AXL::LanguageCode AXL::ResolveLanguageCode(const ResourceHash aResource)
{
    static std::unordered_map<ResourceHash,LanguageCode> s_codes {
        {AXL::ResourcePath(R"(base\localization\ar-ar\onscreens\onscreens.json)"), ("ar-ar")},
        {AXL::ResourcePath(R"(base\localization\cz-cz\onscreens\onscreens.json)"), ("cz-cz")},
        {AXL::ResourcePath(R"(base\localization\db-db\onscreens\onscreens.json)"), ("db-db")},
        {AXL::ResourcePath(R"(base\localization\de-de\onscreens\onscreens.json)"), ("de-de")},
        {AXL::ResourcePath(R"(base\localization\en-us\onscreens\onscreens.json)"), ("en-us")},
        {AXL::ResourcePath(R"(base\localization\es-es\onscreens\onscreens.json)"), ("es-es")},
        {AXL::ResourcePath(R"(base\localization\es-mx\onscreens\onscreens.json)"), ("es-mx")},
        {AXL::ResourcePath(R"(base\localization\fr-fr\onscreens\onscreens.json)"), ("fr-fr")},
        {AXL::ResourcePath(R"(base\localization\hu-hu\onscreens\onscreens.json)"), ("hu-hu")},
        {AXL::ResourcePath(R"(base\localization\it-it\onscreens\onscreens.json)"), ("it-it")},
        {AXL::ResourcePath(R"(base\localization\jp-jp\onscreens\onscreens.json)"), ("jp-jp")},
        {AXL::ResourcePath(R"(base\localization\kr-kr\onscreens\onscreens.json)"), ("kr-kr")},
        {AXL::ResourcePath(R"(base\localization\pl-pl\onscreens\onscreens.json)"), ("pl-pl")},
        {AXL::ResourcePath(R"(base\localization\pt-br\onscreens\onscreens.json)"), ("pt-br")},
        {AXL::ResourcePath(R"(base\localization\ru-ru\onscreens\onscreens.json)"), ("ru-ru")},
        {AXL::ResourcePath(R"(base\localization\th-th\onscreens\onscreens.json)"), ("th-th")},
        {AXL::ResourcePath(R"(base\localization\tr-tr\onscreens\onscreens.json)"), ("tr-tr")},
        {AXL::ResourcePath(R"(base\localization\zh-cn\onscreens\onscreens.json)"), ("zh-cn")},
        {AXL::ResourcePath(R"(base\localization\zh-tw\onscreens\onscreens.json)"), ("zh-tw")},
    };

    return s_codes.contains(aResource) ? s_codes.at(aResource) : "";
}

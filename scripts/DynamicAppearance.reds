module ArchiveXL.DynamicAppearance

public func OverrideDynamicAppearanceCondition(app: String, attr: String, value: String) -> String {
    if !StrContains(app, "!") {
        return app;
    }

    attr += "=";

    let base: String;
    let dynamic: String;
    StrSplitFirst(app, "!", base, dynamic);

    let variant: String;
    let hash: String;
    StrSplitFirst(dynamic, "%", variant, hash);

    let parts = StrSplit(variant, "+", false);
    let n = ArraySize(parts);
    let i = 0;
    while i < n {
        if StrFindFirst(parts[i], attr) == 0 {
            parts[i] = attr + value;
            break;
        }
        i += 1;
    }
    if i >= n {
        ArrayPush(parts, attr + value);
        n += 1;
    }

    dynamic = parts[0];
    let i = 1;
    while i < n {
        dynamic += "+" + parts[i];
        i += 1;
    }

    if StrLen(hash) > 0 {
        dynamic += "%" + hash;
    }

    return base + "!" + dynamic;
}

public func ConvertAppearanceNameToTPP(app: String) -> String {
    if !StrContains(app, "!") {
        return StrReplace(app, "&FPP", "&TPP");
    }

    return OverrideDynamicAppearanceCondition(app, "camera", "tpp");
}

public func ConvertAppearanceNameToFPP(app: String) -> String {
    if !StrContains(app, "!") {
        return StrReplace(app, "&TPP", "&FPP");
    }

    return OverrideDynamicAppearanceCondition(app, "camera", "fpp");
}

public func ConvertAppearanceNameToPartialSleeves(app: String) -> String {
    if !StrContains(app, "!") {
        return StrReplace(app, "&Full", "&Part");
    }

    return OverrideDynamicAppearanceCondition(app, "sleeves", "part");
}

public func ConvertAppearanceNameToFullSleeves(app: String) -> String {
    if !StrContains(app, "!") {
        return StrReplace(app, "&Part", "&Full");
    }

    return OverrideDynamicAppearanceCondition(app, "sleeves", "full");
}

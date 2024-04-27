param ($StageDir, $ProjectName, $Version, $GlobalScope = "${ProjectName}.Global")

$ScriptsDir = "${StageDir}/red4ext/plugins/${ProjectName}/Scripts"

$SourceFiles = Get-ChildItem -Path "scripts" -Filter *.reds -Recurse
$Bundles = @{}

foreach ($ScriptFile in $SourceFiles) {
    $Content = Get-Content $ScriptFile.FullName
    $Module = ($Content | Select-String -Pattern "^module\s+(.+)" -List | %{$_.matches.groups[1].Value})
    $Imports = ($Content | Select-String -Pattern "^import\s+(.+)" -List | %{$_.matches.groups[1].Value})
    $Source = ($Content | Select-String -Pattern "^\s*(//|module\s|import\s)" -NotMatch | Select-String -Pattern "^\s*$" -NotMatch) | Out-String
    $Scope = $Module ?? $GlobalScope

    if ($Bundles[$Scope] -eq $null) {
        $Bundles[$Scope] = @{
            Scope = $Scope
            Module = $Module
            Imports = @{}
            Sources = [System.Collections.ArrayList]@()
        }
    }

    $Bundle = $Bundles[$Scope]
    $Bundle.Sources.Add($Source.Trim()) > $null

    if ($Imports -ne $null) {
        foreach ($Import in $Imports) {
            $Bundle.Imports[$Import] = 1
        }
    }
}

New-Item -ItemType directory -Force -Path ${ScriptsDir} | Out-Null

foreach ($Bundle in $Bundles.Values) {
    $BundleFile = "${ScriptsDir}/$($Bundle.Scope).reds"
    Out-File -FilePath ${BundleFile} -Encoding ascii -InputObject "// ${ProjectName} ${Version}"

    if ($Bundle.Module) {
        Out-File -FilePath ${BundleFile} -Encoding ascii -InputObject "module $($Bundle.Module)" -Append
    }

    foreach ($Import in $Bundle.Imports.Keys) {
        Out-File -FilePath ${BundleFile} -Encoding ascii -InputObject "import ${Import}" -Append
    }

    foreach ($Source in $Bundle.Sources) {
        if ($Source) {
            Out-File -FilePath ${BundleFile} -Encoding ascii -InputObject "`n${Source}" -Append
        }
    }
}

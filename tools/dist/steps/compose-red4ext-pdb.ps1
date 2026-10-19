param ($StageDir, $ReleaseBin, $ProjectName)

$PluginDir = "${StageDir}/red4ext/plugins/${ProjectName}"
$ReleasePdb = ${ReleaseBin} -replace "\.dll$", ".pdb"

New-Item -ItemType directory -Force -Path ${PluginDir} | Out-Null
Copy-Item -Path ${ReleasePdb} -Destination "${PluginDir}/${ProjectName}.pdb"

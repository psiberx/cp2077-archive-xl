param ($StageDir, $ReleaseBin, $ProjectName)

$PluginDir = "${StageDir}/red4ext/plugins/${ProjectName}"

New-Item -ItemType directory -Force -Path ${PluginDir} | Out-Null
Copy-Item -Path ${ReleaseBin} -Destination "${PluginDir}/${ProjectName}.dll"

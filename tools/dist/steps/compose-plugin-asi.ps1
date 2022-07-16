param ($StageDir, $ReleaseBin, $ProjectName)

$PluginDir = "${StageDir}/bin/x64/plugins"

New-Item -ItemType directory -Force -Path ${PluginDir} | Out-Null
Copy-Item -Path ${ReleaseBin} -Destination "${PluginDir}/${ProjectName}.asi"

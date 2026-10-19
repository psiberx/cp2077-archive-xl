param ($StageDir, $ProjectName)

$PluginDir = "${StageDir}/red4ext/plugins/${ProjectName}"

New-Item -ItemType directory -Force -Path ${PluginDir} | Out-Null
Copy-Item -Path "config/Config.yaml" -Destination "${PluginDir}/${ProjectName}.yaml"

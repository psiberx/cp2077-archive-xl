param ($StageDir, $ProjectName)

$ScriptsDir = "${StageDir}/r6/scripts/${ProjectName}"

New-Item -ItemType directory -Force -Path ${ScriptsDir} | Out-Null
Get-Content "support/redscript/*.reds" | Out-File -Encoding ascii "${ScriptsDir}/${ProjectName}.reds"

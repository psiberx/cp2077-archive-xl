param ($StageDir)

$HintsDir = "${StageDir}/r6/config/redsUserHints"

New-Item -ItemType directory -Force -Path ${HintsDir} | Out-Null
Copy-Item -Path "support/hints/*" -Destination ${HintsDir}

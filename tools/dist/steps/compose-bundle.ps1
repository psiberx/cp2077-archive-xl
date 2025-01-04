param ($StageDir, $ReleaseBin, $ProjectName)

$BundleDir = "${StageDir}/red4ext/plugins/${ProjectName}/Bundle"

New-Item -ItemType directory -Force -Path ${BundleDir} | Out-Null
Copy-Item -Path "archive/packed/archive/pc/mod/*" -Destination ${BundleDir}
Copy-Item -Path "archive/source/resources/*" -Destination ${BundleDir}

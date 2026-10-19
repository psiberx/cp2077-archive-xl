param ($StageDir, $ProjectName, $DistDir, $Version, $Suffix = "")

New-Item -ItemType directory -Force -Path ${DistDir} | Out-Null
Compress-Archive -Path "${StageDir}/*" -CompressionLevel Optimal -Force -DestinationPath "${DistDir}/${ProjectName}-${Version}${Suffix}.zip"

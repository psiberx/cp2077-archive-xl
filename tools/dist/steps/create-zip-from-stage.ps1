param ($StageDir, $ProjectName, $DistDir, $GitTag, $Suffix = "")

if ($GitTag -eq $null) {
    $GitTag = git describe --tags
    $GitTag = $GitTag.Substring(1)
}

New-Item -ItemType directory -Force -Path ${DistDir} | Out-Null
Compress-Archive -Path "${StageDir}/*" -CompressionLevel Optimal -Force -DestinationPath "${DistDir}/${ProjectName}-${GitTag}${Suffix}.zip"

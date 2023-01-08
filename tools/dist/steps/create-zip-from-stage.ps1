param ($StageDir, $ProjectName, $DistDir, $Version, $Suffix = "")

if ($Version -eq $null) {
    $Version = Select-String -Path "src/App/Project.hpp" -Pattern """(\d+\.\d+\.\d+)""" -List | % {"$($_.Matches.Groups[1])"}
}

New-Item -ItemType directory -Force -Path ${DistDir} | Out-Null
Compress-Archive -Path "${StageDir}/*" -CompressionLevel Optimal -Force -DestinationPath "${DistDir}/${ProjectName}-${Version}${Suffix}.zip"

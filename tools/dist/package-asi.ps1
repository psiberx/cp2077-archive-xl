param ($ReleaseBin, $ProjectName = "ArchiveXL")

$StageDir = "build/package"
$DistDir = "build/dist"

& $($PSScriptRoot + "\steps\compose-plugin-asi.ps1") -StageDir ${StageDir} -ProjectName ${ProjectName} -ReleaseBin ${ReleaseBin}
& $($PSScriptRoot + "\steps\create-zip-from-stage.ps1") -StageDir ${StageDir} -ProjectName ${ProjectName} -DistDir ${DistDir} -Suffix "-asi"

Remove-Item -Recurse ${StageDir}

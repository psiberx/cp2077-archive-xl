param ($GameDir, $ReleaseBin, $ProjectName = "ArchiveXL")

$StageDir = "build/package"

& $($PSScriptRoot + "\steps\compose-plugin-red4ext.ps1") -StageDir ${StageDir} -ProjectName ${ProjectName} -ReleaseBin ${ReleaseBin}
& $($PSScriptRoot + "\steps\install-from-stage.ps1") -StageDir ${StageDir} -GameDir ${GameDir}

Remove-Item -Recurse ${StageDir}

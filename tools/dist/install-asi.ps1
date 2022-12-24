param ($GameDir, $ReleaseBin, $ProjectName = "ArchiveXL")

$StageDir = "build/package"

& $($PSScriptRoot + "\steps\compose-plugin-asi.ps1") -StageDir ${StageDir} -ProjectName ${ProjectName} -ReleaseBin ${ReleaseBin}
& $($PSScriptRoot + "\steps\compose-redscripts.ps1") -StageDir ${StageDir} -ProjectName ${ProjectName}
& $($PSScriptRoot + "\steps\install-from-stage.ps1") -StageDir ${StageDir} -GameDir ${GameDir}

Remove-Item -Recurse ${StageDir}

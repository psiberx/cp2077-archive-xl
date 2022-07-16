### How to use

There are two scripts for each ASI and RED4ext setup:

- Install: Deploys the current build. Intended to be used as an after build script.
- Package: Prepares the current build as a distribution archive for end users.

### How to use

Examples:

- `.\tools\dist\install-red4ext.ps1 -GameDir 'C:\Games\Cyberpunk 2077' -ReleaseBin 'C:\Dev\ArchiveXL\build\debug\ArchiveXL.dll'`
- `.\tools\dist\package-red4ext.ps1 -ReleaseBin 'C:\Dev\ArchiveXL\build\release\ArchiveXL.dll'`

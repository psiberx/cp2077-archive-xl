Select-String -Path "src/App/Project.hpp" -Pattern """(\d+\.\d+\.\d+)""" -List | %{"$($_.Matches.Groups[1])"} | Write-Output

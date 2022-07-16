param ($StageDir, $GameDir)

Copy-Item -Path "${StageDir}/*" -Recurse -Force -Destination ${GameDir}

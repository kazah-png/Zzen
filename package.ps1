param([string]$Version='0.3.1')
$ErrorActionPreference='Stop'
if($Version -notmatch '^\d+\.\d+(\.\d+)?$'){throw 'Invalid version'}
$root=$PSScriptRoot
$exe=Join-Path $root 'dist/Zzen/Zzen.exe'
if(!(Test-Path $exe)){throw 'Build with build.ps1 -Test first'}
$stage=Join-Path $root ('build/package-'+[guid]::NewGuid().ToString('N'))
$app=Join-Path $stage 'Zzen'
New-Item -ItemType Directory -Path "$app/data","$app/docs","$root/releases" -Force | Out-Null
Copy-Item -LiteralPath $exe -Destination "$app/Zzen.exe"
Copy-Item -LiteralPath "$root/data/catalog.json" -Destination "$app/data/catalog.json"
Copy-Item -LiteralPath "$root/README.md" -Destination "$app/README.md"
Copy-Item -LiteralPath "$root/LICENSE" -Destination "$app/LICENSE"
Copy-Item -LiteralPath "$root/docs/VALIDATION.md" -Destination "$app/docs/VALIDATION.md"
Get-ChildItem -LiteralPath "$root/docs" -Filter *.png | Copy-Item -Destination "$app/docs"
$zip=Join-Path $root "releases/Zzen-$Version-Windows-x64.zip"
Compress-Archive -LiteralPath $app -DestinationPath $zip -Force
$hash=(Get-FileHash $zip -Algorithm SHA256).Hash.ToLowerInvariant()
[IO.File]::WriteAllText("$zip.sha256","$hash  $([IO.Path]::GetFileName($zip))`n",[Text.UTF8Encoding]::new($false))
Write-Output $zip
Write-Output "SHA256 $hash"

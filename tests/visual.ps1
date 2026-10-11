param(
 [Parameter(Mandatory=$true)][string]$Fixtures,
 [string]$Executable=(Join-Path (Split-Path $PSScriptRoot) 'dist/Zzen/Zzen.exe'),
 [string]$RewardsCapture
)
$ErrorActionPreference='Stop'
$reports=Join-Path (Split-Path $PSScriptRoot) 'build/visual-reports'
New-Item -ItemType Directory -Path $reports -Force | Out-Null
$cases=@(
 @{Image='formation.png';Contains='Alineación · Terminar edición'},
 @{Image='difficulty.png';Contains='Dificultad'},
 @{Image='difficulty-live.png';Contains='Dificultad'},
 @{Image='chronicle-1-top.png';Contains='Occult'},
 @{Image='score-5-0.png';Contains='SCORE=5-0'},
 @{Image='score-4-0.png';Contains='SCORE=4-0'},
 @{Image='score-4-0-cursor.png';Contains='SCORE=4-0'},
 @{Image='score-transition.png';Contains='SCORE=unreadable'},
 @{Image='duel-defeat.png';Excludes='Marcador final'}
)
if($RewardsCapture){$cases+=@{Image=(Resolve-Path $RewardsCapture).Path;Contains='Recompensas'}}
foreach($case in $cases){
 $image=if([IO.Path]::IsPathRooted($case.Image)){$case.Image}else{Join-Path (Resolve-Path $Fixtures) $case.Image}
 $report=Join-Path $reports ([IO.Path]::GetFileName($case.Image)+'.txt')
 $process=Start-Process -FilePath $Executable -ArgumentList @('--analyze',('"'+$image+'"'),('"'+$report+'"')) -PassThru -WindowStyle Hidden
 if(!$process.WaitForExit(30000)){throw "Timeout: $($case.Image)"}
 if($process.ExitCode -ne 0){throw "Error: $($case.Image)"}
 $text=Get-Content -LiteralPath $report -Raw -Encoding utf8
 if($case['Contains'] -and !$text.Contains($case['Contains'])){throw "Lectura incorrecta: $($case.Image)"}
 if($case['Excludes'] -and $text.Contains($case['Excludes'])){throw "Falso positivo: $($case.Image)"}
 Write-Output "OK $($case.Image)"
}

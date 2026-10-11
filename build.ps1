param([switch]$Test,[switch]$TestsOnly,[string]$OutputDirectory='dist/Zzen')
$ErrorActionPreference='Stop'
$base=$PSScriptRoot
$vswhere='C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe'
$vs=& $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if(!$vs){throw 'Visual Studio C++ build tools are required.'}
$vc=(Get-ChildItem "$vs\VC\Tools\MSVC" -Directory | Sort-Object Name -Descending | Select-Object -First 1).FullName
$sdk='C:\Program Files (x86)\Windows Kits\10'
$ver=(Get-ChildItem "$sdk\Include" -Directory | Sort-Object Name -Descending | Select-Object -First 1).Name
$inc=@("$vc\include","$sdk\Include\$ver\ucrt","$sdk\Include\$ver\um","$sdk\Include\$ver\shared","$sdk\Include\$ver\cppwinrt") | ForEach-Object { '/I'+$_ }
$libs=@("$vc\lib\x64","$sdk\Lib\$ver\ucrt\x64","$sdk\Lib\$ver\um\x64") | ForEach-Object { '/LIBPATH:'+$_ }
New-Item -ItemType Directory -Path "$base\build" -Force | Out-Null
$compilerPathBefore=$env:PATH
$env:PATH="$sdk\bin\$ver\x64;$env:PATH"
try {
 $out=Join-Path $base $OutputDirectory
 New-Item -ItemType Directory -Path "$out/data" -Force | Out-Null
 if(!$TestsOnly){
 & "$sdk/bin/$ver/x64/rc.exe" /nologo /i "$base/assets" /i "$sdk/Include/$ver/um" /i "$sdk/Include/$ver/shared" /fo "$base/build/zzen.res" "$base/src/app.rc"
 if($LASTEXITCODE -ne 0){throw "Resource compilation failed"}
 & "$vc\bin\Hostx64\x64\cl.exe" /nologo /std:c++20 /EHsc /W4 /utf-8 /DUNICODE /D_UNICODE /DNOMINMAX /O2 /MT @inc "$base/src/zzen.cpp" "$base/build/zzen.res" "/Fo:$base/build/zzen.obj" "/Fe:$out/Zzen.exe" /link /MANIFEST:EMBED /SUBSYSTEM:WINDOWS @libs windowsapp.lib runtimeobject.lib gdiplus.lib comctl32.lib user32.lib gdi32.lib ole32.lib shell32.lib comdlg32.lib dwmapi.lib uxtheme.lib
 if($LASTEXITCODE -ne 0){throw 'Zzen compilation failed.'}
 }
 Copy-Item "$base/data/catalog.json" "$out/data/catalog.json" -Force
 if(Test-Path "$base/README.md"){Copy-Item "$base/README.md" "$out/README.md" -Force}
 if($Test -or $TestsOnly){
  foreach($name in @('core_tests','storage_tests','engine_tests','i18n_tests')){
   & "$vc\bin\Hostx64\x64\cl.exe" /nologo /std:c++20 /EHsc /W4 /utf-8 /DUNICODE /D_UNICODE /DNOMINMAX /MT @inc "$base/tests/$name.cpp" "/Fo:$base/build/$name.obj" "/Fe:$base/build/$name.exe" /link @libs windowsapp.lib runtimeobject.lib gdiplus.lib user32.lib gdi32.lib ole32.lib
   if($LASTEXITCODE -ne 0){throw "Compilation failed: $name"}
   & "$base/build/$name.exe" "$base/build/test-data-$name"
   if($LASTEXITCODE -ne 0){throw "Test failed: $name"}
  }
 }
} finally { $env:PATH=$compilerPathBefore }

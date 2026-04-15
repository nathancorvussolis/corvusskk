@echo off
setlocal
pushd "%~dp0"

call _vsdev.cmd

call _env.cmd



rem > _sign.cmd  <SHA-1> <URL>
rem     * <SHA-1> : SHA-1 thumbprint of certificate
rem     * <URL> : RFC-3161 timestamp server

set DESCRIPTION="CorvusSKK"

set SHA1HASH=%1
set TIMESTAMPSERVER=%2

set SIGNCOMMAND=signtool sign /v /d %DESCRIPTION% /sha1 %SHA1HASH% /fd sha256 /tr %TIMESTAMPSERVER% /td sha256



set BINFILES=
rem x86
set BINFILES=%BINFILES% "..\build\Win32\Release\*.dll" "..\build\Win32\Release\*.exe"
rem x64
set BINFILES=%BINFILES% "..\build\x64\Release\*.dll" "..\build\x64\Release\*.exe"
rem ARM64EC   TIP only
set BINFILES=%BINFILES% "..\build\ARM64EC\Release\*.dll"
rem ARM64
set BINFILES=%BINFILES% "..\build\ARM64\Release\*.dll" "..\build\ARM64\Release\*.exe"

set MSIFILES=
rem x86
set MSIFILES=%MSIFILES% "%OutDir%\x86.msi"
rem x64
set MSIFILES=%MSIFILES% "%OutDir%\x64.msi"
rem ARM64
set MSIFILES=%MSIFILES% "%OutDir%\arm64.msi"

rem bundle
set BEFILE="%OutDir%\corvusskk-%VERSION%-engine.exe"
set BSFILE="%OutDir%\corvusskk-%VERSION%.exe"



dotnet tool restore

call _clean.cmd

echo sign binary files
%SIGNCOMMAND% %BINFILES%

call _build_msi.cmd

echo sign msi files
%SIGNCOMMAND% %MSIFILES%

call _build_bundle.cmd

echo detach engine
dotnet wix burn detach %BSFILE% -engine %BEFILE%

echo sign engine
%SIGNCOMMAND% %BEFILE%

echo reattach engine
dotnet wix burn reattach %BSFILE% -engine %BEFILE%

echo sign bundle
%SIGNCOMMAND% %BSFILE%



popd
endlocal

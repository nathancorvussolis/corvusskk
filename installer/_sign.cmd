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
rem ARM64
set BINFILES=%BINFILES% "..\build\ARM64\Release\*.dll" "..\build\ARM64\Release\*.exe"
rem ARM64EC   TIP only
set BINFILES=%BINFILES% "..\build\ARM64EC\Release\*.dll"



call _clean.cmd

echo sign binary files
%SIGNCOMMAND% %BINFILES%

set SignOutput=true

call _build_msi.cmd

call _build_bundle.cmd



popd
endlocal

@echo off
setlocal
pushd "%~dp0"

call _clean.cmd

call _vsdev.cmd

call _env.cmd

call _build_doc.cmd



if "%1"=="" goto usage
if "%2"=="" goto usage

rem > _sign.cmd  <SHA-1> <URL>
rem     * <SHA-1> : SHA-1 thumbprint of certificate
rem     * <URL> : RFC-3161 timestamp server

set DESCRIPTION=CorvusSKK

set SHA1HASH=%1
set TIMESTAMPSERVER=%2

set SIGNCOMMANDBIN=signtool sign /v /d "%DESCRIPTION%"   /sha1 %SHA1HASH% /fd sha256 /tr %TIMESTAMPSERVER% /td sha256
set SIGNCOMMANDISS=signtool sign /v /d $q%DESCRIPTION%$q /sha1 %SHA1HASH% /fd sha256 /tr %TIMESTAMPSERVER% /td sha256 $f
set COPYCOMMANDISS=xcopy $f $q%~dp0%OUTDIR%\tmp\$q /y /i



set BINFILES=
rem x86
set BINFILES=%BINFILES% "..\build\Win32\Release\*.dll" "..\build\Win32\Release\*.exe"
rem x64
set BINFILES=%BINFILES% "..\build\x64\Release\*.dll" "..\build\x64\Release\*.exe"
rem ARM64EC   TIP only
set BINFILES=%BINFILES% "..\build\ARM64EC\Release\*.dll"
rem ARM64
set BINFILES=%BINFILES% "..\build\ARM64\Release\*.dll" "..\build\ARM64\Release\*.exe"

%SIGNCOMMANDBIN% %BINFILES%



set PATH=%PATH%;%ProgramFiles%\Inno Setup 7
set PATH=%PATH%;%ProgramFiles(x86)%\Inno Setup 7
set PATH=%PATH%;%ProgramFiles(x86)%\Inno Setup 6

ISCC.exe /DSIGN /SMySignTool="%SIGNCOMMANDISS%" /SMyCopyTool="%COPYCOMMANDISS%" installer.iss

copy /b /y "%OUTDIR%\tmp\uninst.e32.tmp" "%OUTDIR%"

rd /s /q "%OUTDIR%\tmp"



goto end

:usage
echo Usage: _sign.cmd  ^<SHA-1^> ^<URL^>
echo   * ^<SHA-1^> : SHA-1 thumbprint of certificate
echo   * ^<URL^> : RFC-3161 timestamp server

:end
popd
endlocal

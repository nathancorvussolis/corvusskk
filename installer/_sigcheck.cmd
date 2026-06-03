@echo off
setlocal
pushd "%~dp0"

call _env.cmd



where /q sigcheck.exe
if %ERRORLEVEL% equ 1 (
  pushd "%OUTDIR%"
  curl -O -R "https://live.sysinternals.com/sigcheck.exe"
  popd
)

set PATH=%PATH%;%~dp0%OUTDIR%

set BINFILES=
rem x86
set BINFILES=%BINFILES% "..\build\Win32\Release\*.dll" "..\build\Win32\Release\*.exe"
rem x64
set BINFILES=%BINFILES% "..\build\x64\Release\*.dll" "..\build\x64\Release\*.exe"
rem ARM64EC   TIP only
set BINFILES=%BINFILES% "..\build\ARM64EC\Release\*.dll"
rem ARM64
set BINFILES=%BINFILES% "..\build\ARM64\Release\*.dll" "..\build\ARM64\Release\*.exe"
rem Uninstaller, Installer
set BINFILES=%BINFILES% "%OUTDIR%\uninst.e32.tmp" "%OUTDIR%\corvusskk-%VERSION%.exe"

for %%i in (%BINFILES%) do sigcheck.exe -vs %%i



popd
endlocal

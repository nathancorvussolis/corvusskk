@echo off

set VSWHEREEXE="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
set VSWHEREOPT=-version [17^^,18^^) -requires Microsoft.Component.MSBuild -property installationPath

for /f "usebackq tokens=*" %%i in (`%VSWHEREEXE% %VSWHEREOPT%`) do (
  set VSDEVCMDBAT=%%i\Common7\Tools\VsDevCmd.bat
)

if exist "%VSDEVCMDBAT%" (
  call "%VSDEVCMDBAT%" %* > nul
)

set VSWHEREEXE=
set VSWHEREOPT=
set VSDEVCMDBAT=

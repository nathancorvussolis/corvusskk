@echo off

set VSWHERE="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
set VSWHEREOPT=-version ^^^[17.0^^^,18.0^^^) -requires Microsoft.Component.MSBuild -property installationPath

for /f "usebackq tokens=*" %%i in (`%VSWHERE% %VSWHEREOPT%`) do (
  if exist "%%i" (
    set VS2022InstallDir=%%i
  )
)

if not exist "%VS2022InstallDir%\Common7\Tools\VsDevCmd.bat" exit /B 1

call "%VS2022InstallDir%\Common7\Tools\VsDevCmd.bat" > nul

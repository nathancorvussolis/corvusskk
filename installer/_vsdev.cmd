@echo off

for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -requires Microsoft.Component.MSBuild -property installationPath`) do (
  if exist "%%i" (
    set VS2017InstallDir=%%i
  )
)

if not exist "%VS2017InstallDir%\Common7\Tools\VsDevCmd.bat" exit /B 1

call "%VS2017InstallDir%\Common7\Tools\VsDevCmd.bat"

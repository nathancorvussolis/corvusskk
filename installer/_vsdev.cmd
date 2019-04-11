@echo off

for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -version ^[16.0^,17.0^) -requires Microsoft.Component.MSBuild -property installationPath`) do (
  if exist "%%i" (
    set VS2019InstallDir=%%i
  )
)

if not exist "%VS2019InstallDir%\Common7\Tools\VsDevCmd.bat" exit /B 1

call "%VS2019InstallDir%\Common7\Tools\VsDevCmd.bat"

@echo off
setlocal

pushd "%~dp0"

if not exist "%USERPROFILE%\.dotnet\tools\.store\wix" (
  dotnet tool install --global wix
)

echo [Tool]
wix --version

set EXTENSIONS= ^
WixToolset.Bal.wixext ^
WixToolset.UI.wixext ^
WixToolset.Util.wixext

for %%i in (%EXTENSIONS%) do (
  if not exist "%USERPROFILE%\.wix\extensions\%%i" (
    wix extension add --global %%i
  )
)

echo [Extensions]
wix extension list --global

popd

endlocal

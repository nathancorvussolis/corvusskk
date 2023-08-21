@echo off
setlocal

pushd "%~dp0"

if not exist "%USERPROFILE%\.dotnet\tools\wix.exe" (
  dotnet tool install --global wix
)

if not exist "%USERPROFILE%\.wix\extensions\WixToolset.UI.wixext" (
  wix extension add --global WixToolset.UI.wixext
)

if not exist "%USERPROFILE%\.wix\extensions\WixToolset.Bal.wixext" (
  wix extension add --global WixToolset.Bal.wixext
)

if not exist "%USERPROFILE%\.wix\extensions\WixToolset.Util.wixext" (
  wix extension add --global WixToolset.Util.wixext
)

popd

endlocal

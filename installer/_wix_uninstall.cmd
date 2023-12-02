@echo off
setlocal

pushd "%~dp0"

set EXTENSIONS= ^
WixToolset.Bal.wixext ^
WixToolset.UI.wixext ^
WixToolset.Util.wixext

for %%i in (%EXTENSIONS%) do (
  wix extension remove --global %%i
)

dotnet tool uninstall --global wix

popd

endlocal

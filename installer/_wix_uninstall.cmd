@echo off
setlocal

pushd "%~dp0"

wix extension remove --global WixToolset.UI.wixext

wix extension remove --global WixToolset.Bal.wixext

wix extension remove --global WixToolset.Util.wixext

dotnet tool uninstall --global wix

popd

endlocal

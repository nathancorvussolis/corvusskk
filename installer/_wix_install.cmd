@echo off
setlocal

pushd "%~dp0"

dotnet tool update --global wix

echo [Tool]
wix --version

set EXTENSIONS= ^
WixToolset.Bal.wixext ^
WixToolset.ComPlus.wixext ^
WixToolset.Dependency.wixext ^
WixToolset.DifxApp.wixext ^
WixToolset.DirectX.wixext ^
WixToolset.Firewall.wixext ^
WixToolset.Http.wixext ^
WixToolset.Iis.wixext ^
WixToolset.Msmq.wixext ^
WixToolset.Netfx.wixext ^
WixToolset.PowerShell.wixext ^
WixToolset.Sql.wixext ^
WixToolset.UI.wixext ^
WixToolset.Util.wixext ^
WixToolset.VisualStudio.wixext

for %%i in (%EXTENSIONS%) do (
  wix extension remove --global %%i
  wix extension add --global %%i
)

echo [Extensions]
wix extension list --global

popd

endlocal

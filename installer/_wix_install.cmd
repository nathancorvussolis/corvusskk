@echo off
setlocal

pushd "%~dp0"

set WIXTOOLSET_VERSION=4.0.5

dotnet tool update wix --global --version %WIXTOOLSET_VERSION%

echo [Tool]
wix --version

set WIXTOOLSET_EXTENSIONS= ^
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

for %%i in (%WIXTOOLSET_EXTENSIONS%) do (
  wix extension remove --global %%i
  wix extension add --global %%i/%WIXTOOLSET_VERSION%
)

echo [Extensions]
wix extension list --global

popd

endlocal

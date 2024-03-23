@echo off
setlocal

pushd "%~dp0"

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
)

dotnet tool uninstall wix --global

popd

endlocal

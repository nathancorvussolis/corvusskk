@echo off
setlocal

pushd "%~dp0"

call _vsdev.cmd

call _version.cmd

wix extension add WixToolset.Bal.wixext
wix extension add WixToolset.Util.wixext

echo build bundle

wix build -arch x86 ^
-ext WixToolset.Bal.wixext -ext WixToolset.Util.wixext ^
installer-bundle.wxs -out "%TARGETDIR%\corvusskk-%VERSION%.exe"

popd

endlocal

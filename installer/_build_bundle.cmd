@echo off
setlocal

pushd "%~dp0"

call _vsdev.cmd

call _version.cmd

wix extension --nologo add WixToolset.Bal.wixext
wix extension --nologo add WixToolset.Util.wixext

echo build bundle

wix build --nologo -arch x86 ^
-ext WixToolset.Bal.wixext -ext WixToolset.Util.wixext ^
installer-bundle.wxs -out "%TARGETDIR%\corvusskk-%VERSION%.exe"

popd

endlocal

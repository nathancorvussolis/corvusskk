@echo off
setlocal

pushd %~dp0

call _version.cmd
set TARGETDIR=build

"%WIX%bin\candle.exe" corvusskk-x86.wxs -nologo -out "%TARGETDIR%\corvusskk-%VERSION%-x86.wixobj"
"%WIX%bin\light.exe" "%TARGETDIR%\corvusskk-%VERSION%-x86.wixobj" -nologo -out "%TARGETDIR%\corvusskk-%VERSION%-x86.msi" -ext WixUIExtension -sw1056

popd

endlocal

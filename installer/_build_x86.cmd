@echo off
setlocal
set TARGETDIR=build

pushd %~dp0

"%WIX%bin\candle.exe" corvusskk-x86.wxs -nologo -out "%TARGETDIR%\corvusskk-x86.wixobj"
"%WIX%bin\light.exe" "%TARGETDIR%\corvusskk-x86.wixobj" -nologo -out "%TARGETDIR%\corvusskk-x86.msi" -ext WixUIExtension -sw1056

popd

endlocal

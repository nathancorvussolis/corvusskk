@echo off
setlocal
set TARGETDIR=build

pushd %~dp0

"%WIX%bin\candle.exe" corvusskk-x64.wxs -nologo -out %TARGETDIR%\corvusskk-x64.wixobj
"%WIX%bin\light.exe" %TARGETDIR%\corvusskk-x64.wixobj -nologo -out %TARGETDIR%\corvusskk-x64.msi -ext WixUIExtension -sw1056

popd

endlocal

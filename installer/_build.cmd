@echo off

pushd %~dp0

"%WIX%bin\candle.exe" corvusskk-x64.wxs -nologo
"%WIX%bin\light.exe" corvusskk-x64.wixobj -nologo -ext WixUIExtension -sw1056

"%WIX%bin\candle.exe" corvusskk-x86.wxs -nologo
"%WIX%bin\light.exe" corvusskk-x86.wixobj -nologo -ext WixUIExtension -sw1056

popd

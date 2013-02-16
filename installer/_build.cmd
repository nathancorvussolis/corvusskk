@echo off

pushd %~dp0

"%WIX%bin\candle.exe" -nologo "corvusskk-x64.wxs"
"%WIX%bin\light.exe" -nologo -ext WixUIExtension "corvusskk-x64.wixobj"

"%WIX%bin\candle.exe" -nologo "corvusskk-x86.wxs"
"%WIX%bin\light.exe" -nologo -ext WixUIExtension "corvusskk-x86.wixobj"

popd

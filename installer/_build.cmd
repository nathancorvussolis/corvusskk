@echo off

pushd %~dp0

rem call _clean.cmd

"%WIX%bin\candle.exe" "corvusskk-x64.wxs" -ext WixUIExtension
"%WIX%bin\light.exe" "corvusskk-x64.wixobj" -ext WixUIExtension
rem "%WIX%bin\light.exe" "corvusskk-x64.wixobj" -ext WixUIExtension -cultures:ja-jp

"%WIX%bin\candle.exe" "corvusskk-x86.wxs" -ext WixUIExtension
"%WIX%bin\light.exe" "corvusskk-x86.wixobj" -ext WixUIExtension
rem "%WIX%bin\light.exe" "corvusskk-x86.wixobj" -ext WixUIExtension -cultures:ja-jp

rem call _mkarc.cmd

popd

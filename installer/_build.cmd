@echo off

pushd %~dp0

call _clean.cmd

"%WIX%bin\candle.exe" "corvusskk-x64.wxs" -ext WixUIExtension
"%WIX%bin\light.exe" "corvusskk-x64.wixobj" -ext WixUIExtension
rem "%WIX%bin\light.exe" "corvusskk-x64.wixobj" -ext WixUIExtension -cultures:ja-jp
7za.exe a corvusskk-x64.7z corvusskk-x64.msi README.TXT

"%WIX%bin\candle.exe" "corvusskk-x86.wxs" -ext WixUIExtension
"%WIX%bin\light.exe" "corvusskk-x86.wixobj" -ext WixUIExtension
rem "%WIX%bin\light.exe" "corvusskk-x86.wixobj" -ext WixUIExtension -cultures:ja-jp
7za.exe a corvusskk-x86.7z corvusskk-x86.msi README.TXT

popd

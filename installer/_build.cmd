@echo off

pushd %~dp0

"%WIX%bin\candle.exe" "corvusskk_x64.wxs" -ext WixUIExtension
"%WIX%bin\light.exe" "corvusskk_x64.wixobj" -ext WixUIExtension
rem "%WIX%bin\light.exe" "corvusskk_x64.wixobj" -ext WixUIExtension -cultures:ja-jp
7za.exe a corvusskk_x64.7z corvusskk_x64.msi README.TXT

"%WIX%bin\candle.exe" "corvusskk_x86.wxs" -ext WixUIExtension
"%WIX%bin\light.exe" "corvusskk_x86.wixobj" -ext WixUIExtension
rem "%WIX%bin\light.exe" "corvusskk_x86.wixobj" -ext WixUIExtension -cultures:ja-jp
7za.exe a corvusskk_x86.7z corvusskk_x86.msi README.TXT

popd

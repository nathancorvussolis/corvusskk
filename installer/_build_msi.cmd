@echo off
setlocal

pushd "%~dp0"

call _vsdev.cmd

call _version.cmd

call _build_sub.cmd

rem x86

"%WIX%bin\candle.exe" -nologo -arch x86 ^
installer-x86.wxs -out "%TARGETDIR%\x86.wixobj"

"%WIX%bin\light.exe" -nologo -ext WixUIExtension -sw1076 ^
"%TARGETDIR%\x86.wixobj" -out "%TARGETDIR%\x86.msi"

rem x64

"%WIX%bin\candle.exe" -nologo -arch x64 ^
installer-x64.wxs -out "%TARGETDIR%\x64.wixobj"

"%WIX%bin\light.exe" -nologo -ext WixUIExtension -sw1076 ^
"%TARGETDIR%\x64.wixobj" -out "%TARGETDIR%\x64.msi"

rem ARM

"%WIX%bin\candle.exe" -nologo -arch x64 ^
installer-arm.wxs -out "%TARGETDIR%\arm.wixobj"

"%WIX%bin\light.exe" -nologo -ext WixUIExtension -sw1076 ^
"%TARGETDIR%\arm.wixobj" -out "%TARGETDIR%\arm.msi"

msiinfo "%TARGETDIR%\arm.msi" /nologo /p Arm64;1033

popd

endlocal

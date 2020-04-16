@echo off
setlocal

pushd "%~dp0"

call _vsdev.cmd

call _version.cmd

call _build_sub.cmd

rem x86

"%WIX%bin\candle.exe" installer-x86.wxs ^
-nologo -out "%TARGETDIR%\x86.wixobj"

"%WIX%bin\light.exe" "%TARGETDIR%\x86.wixobj" ^
-nologo -out "%TARGETDIR%\x86.msi" -ext WixUIExtension -sw1076

rem x64

"%WIX%bin\candle.exe" installer-x64.wxs ^
-nologo -out "%TARGETDIR%\x64.wixobj"

"%WIX%bin\light.exe" "%TARGETDIR%\x64.wixobj" ^
-nologo -out "%TARGETDIR%\x64.msi" -ext WixUIExtension -sw1076

rem ARM

"%WIX%bin\candle.exe" installer-arm.wxs ^
-nologo -out "%TARGETDIR%\arm.wixobj"

"%WIX%bin\light.exe" "%TARGETDIR%\arm.wixobj" ^
-nologo -out "%TARGETDIR%\arm.msi" -ext WixUIExtension -sw1076

msiinfo "%TARGETDIR%\arm.msi" /p Arm64;1033 /nologo

popd

endlocal

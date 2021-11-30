@echo off
setlocal

pushd "%~dp0"

call _vsdev.cmd

call _version.cmd

"%WIX%bin\candle.exe" installer-bundle.wxs ^
-nologo -out "%TARGETDIR%\corvusskk-%VERSION%.wixobj" ^
-ext WixBalExtension -ext WixUtilExtension

"%WIX%bin\light.exe" "%TARGETDIR%\corvusskk-%VERSION%.wixobj" ^
-nologo -out "%TARGETDIR%\corvusskk-%VERSION%.exe" ^
-ext WixBalExtension -ext WixUtilExtension

popd

endlocal

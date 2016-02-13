@echo off
setlocal

pushd "%~dp0"

call _version.cmd

"%WIX%bin\candle.exe" installer-x64.wxs -nologo -out "%TARGETDIR%\x64.wixobj"
"%WIX%bin\light.exe" "%TARGETDIR%\x64.wixobj" -nologo -out "%TARGETDIR%\x64.msi" -sw1056 -sw1076

popd

endlocal

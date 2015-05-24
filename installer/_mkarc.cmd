@echo off
setlocal

pushd %~dp0

call _version.cmd
set TARGETDIR=build

del "%TARGETDIR%\config-sample.zip"

pushd config-sample
7za.exe a -tzip -mtc=off "..\%TARGETDIR%\config-sample.zip" *.xml *.txt
popd

del "%TARGETDIR%\config-lua.zip"

pushd config-lua
7za.exe a -tzip -mtc=off "..\%TARGETDIR%\config-lua.zip" *.lua
popd

del "%TARGETDIR%\corvusskk.zip"

pushd "%TARGETDIR%"
7za.exe a -tzip -mtc=off corvusskk-%VERSION%.zip corvusskk-*.msi ..\README.TXT ..\..\LICENSE.TXT config-sample.zip config-lua.zip
popd

popd

endlocal

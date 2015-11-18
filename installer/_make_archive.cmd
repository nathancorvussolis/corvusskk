@echo off
setlocal

pushd %~dp0

call _version.cmd
set TARGETDIR=build

del "%TARGETDIR%\config-sample.zip"

pushd config-sample
7za.exe a -tzip -mtc=off "..\%TARGETDIR%\config-sample.zip" *.xml *.txt
popd

del "%TARGETDIR%\corvusskk-%VERSION%.zip"

pushd "%TARGETDIR%"
7za.exe a -tzip -mtc=off corvusskk-%VERSION%.zip corvusskk-%VERSION%-x86.msi corvusskk-%VERSION%-x64.msi ..\README.TXT ..\..\LICENSE.TXT config-sample.zip
popd

popd

endlocal

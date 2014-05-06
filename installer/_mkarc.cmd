@echo off
setlocal
set TARGETDIR=build

pushd %~dp0

del "%TARGETDIR%\config-sample.zip"

pushd config-sample
7za.exe a -tzip "..\%TARGETDIR%\config-sample.zip" *.xml *.txt
popd

del %TARGETDIR%\config-lua.zip

pushd config-lua
7za.exe a -tzip "..\%TARGETDIR%\config-lua.zip" *.lua
popd

pushd %TARGETDIR%
7za.exe a -tzip corvusskk.zip corvusskk-x64.msi corvusskk-x86.msi ..\README.TXT ..\..\LICENSE.TXT config-sample.zip config-lua.zip
popd

popd

endlocal

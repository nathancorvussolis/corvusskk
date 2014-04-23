@echo off
setlocal
set TARGETDIR=build

pushd %~dp0

pushd config-sample
7za.exe a -tzip ..\config-sample.zip *.xml *.txt
popd

pushd %TARGETDIR%
7za.exe a -tzip corvusskk.zip corvusskk-x64.msi corvusskk-x86.msi ..\README.TXT ..\..\LICENSE.TXT ..\config-sample.zip
popd

del config-sample.zip

popd

endlocal

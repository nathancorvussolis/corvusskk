@echo off

pushd %~dp0

7za.exe a corvusskk.zip corvusskk-x64.msi corvusskk-x86.msi README.TXT ..\LICENSE.TXT config-sample

popd

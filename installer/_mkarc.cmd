@echo off

pushd %~dp0

7za.exe a corvusskk.7z corvusskk-x64.msi corvusskk-x86.msi README.TXT ..\LICENSE.TXT config-sample

popd

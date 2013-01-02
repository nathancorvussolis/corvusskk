@echo off

pushd %~dp0

pushd config-sample
7za.exe a config-sample.7z *
move config-sample.7z ..\
popd

7za.exe a corvusskk.7z corvusskk-x64.msi corvusskk-x86.msi README.TXT ..\LICENSE.TXT config-sample.7z

popd

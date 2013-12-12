@echo off

pushd %~dp0

pushd config-sample
7za.exe a -tzip ..\config-sample.zip *.xml *.txt
popd

7za.exe a -tzip corvusskk.zip corvusskk-x64.msi corvusskk-x86.msi README.TXT ..\LICENSE.TXT config-sample.zip

del config-sample.zip

popd

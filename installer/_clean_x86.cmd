@echo off
setlocal
set TARGETDIR=build

pushd %~dp0

del "%TARGETDIR%\corvusskk-x86.*"

popd

endlocal

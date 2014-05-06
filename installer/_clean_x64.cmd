@echo off
setlocal
set TARGETDIR=build

pushd %~dp0

del "%TARGETDIR%\corvusskk-x64.*"

popd

endlocal

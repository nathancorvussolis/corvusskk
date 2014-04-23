@echo off
setlocal
set TARGETDIR=build

pushd %~dp0

rd /s /q %TARGETDIR%

popd

endlocal

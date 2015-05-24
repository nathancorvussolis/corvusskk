@echo off
setlocal

pushd %~dp0

set TARGETDIR=build

del "%TARGETDIR%\*.zip"

call _clean_x64.cmd
call _clean_x86.cmd

popd

endlocal

@echo off
setlocal
set TARGETDIR=build

pushd %~dp0

del "%TARGETDIR%\*.zip"
call _clean_x64.cmd
call _clean_x86.cmd

popd

endlocal

@echo off
setlocal

pushd %~dp0

call _version.cmd

call _clean_x86.cmd
call _clean_x64.cmd

del "%TARGETDIR%\*.zip"

popd

endlocal

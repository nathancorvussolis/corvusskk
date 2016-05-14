@echo off
setlocal

pushd "%~dp0"

call _version.cmd

if exist "%TARGETDIR%\x86.*" del "%TARGETDIR%\x86.*"

call _clean_sub.cmd

popd

endlocal

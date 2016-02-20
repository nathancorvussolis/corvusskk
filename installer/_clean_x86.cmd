@echo off
setlocal

pushd "%~dp0"

call _version.cmd

if exist "%TARGETDIR%\README.html" del "%TARGETDIR%\README.html"
if exist "%TARGETDIR%\x86.*" del "%TARGETDIR%\x86.*"

popd

endlocal

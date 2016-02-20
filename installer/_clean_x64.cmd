@echo off
setlocal

pushd "%~dp0"

call _version.cmd

if exist "%TARGETDIR%\x64.*" del "%TARGETDIR%\x64.*"

popd

endlocal

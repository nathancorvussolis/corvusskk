@echo off
setlocal

pushd "%~dp0"

call _version.cmd

del "%TARGETDIR%\x64.*"

popd

endlocal
